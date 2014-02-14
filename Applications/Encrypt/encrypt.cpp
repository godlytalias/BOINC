//Modified by Godly T.Alias

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California


#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#endif

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"


using std::string;

#define CHECKPOINT_FILE "encrypt_state"
#define INPUT_FILENAME "in"
#define OUTPUT_FILENAME "out"

bool run_slow = false;
bool early_exit = false;
bool early_crash = false;
bool early_sleep = false;
double cpu_time=20,comp_result;

// do a billion floating-point ops
// (note: I needed to add an arg to this;
// otherwise the MS C++ compiler optimizes away
// all but the first call to it!)
//

char* rsaencrypt(char msg[],int key,int n) 
{ 
int len,pt,i,j,k,en[1025];
i=0; 

len=strlen(msg); 
while(i!=len) 
{ 
    pt=(int)msg[i];
    k=1; 
    for(j=0;j<key;j++) 
    { 
        k=k*pt; 
        k=k%n; 
    } 
    en[i]=k;
    i++; 
} 
en[i]=-1; 
for(i=0;en[i]!=-1;i++) 
msg[i]=(char)en[i];
msg[i]='\0';
return msg;
}

int do_checkpoint(MFILE& mf, int nchars) {
    int retval;
    string resolved_name;

    FILE* f = fopen("temp", "w");
    if (!f) return 1;
    fprintf(f, "%d", nchars);
    fclose(f);

    retval = mf.flush();
    if (retval) return retval;
    boinc_resolve_filename_s(CHECKPOINT_FILE, resolved_name);
    retval = boinc_rename("temp", resolved_name.c_str());
    if (retval) return retval;

    return 0;
}


int main(int argc, char **argv) {
    int i,p,q,pq;
    int c, nchars = 0, retval, n;
    double fsize, fd;
    char input_path[512], output_path[512], chkpt_path[512], buf[256],sentence[1025];
    MFILE out;
    FILE* state, *infile;

    for (i=0; i<argc; i++) {
        if (!strcmp(argv[i], "-early_exit")) early_exit = true;
        if (!strcmp(argv[i], "-early_crash")) early_crash = true;
        if (!strcmp(argv[i], "-early_sleep")) early_sleep = true;
        if (!strcmp(argv[i], "-run_slow")) run_slow = true;
        if (!strcmp(argv[i], "-cpu_time")) {
            cpu_time = atof(argv[++i]);
        }
    }

    retval = boinc_init();
    if (retval) {
        fprintf(stderr, "%s boinc_init returned %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        exit(retval);
    }

    // open the input file (resolve logical name first)
    //
    boinc_resolve_filename(INPUT_FILENAME, input_path, sizeof(input_path));
    infile = boinc_fopen(input_path, "r");
    if (!infile) {
        fprintf(stderr,
            "%s Couldn't find input file, resolved name %s.\n",
            boinc_msg_prefix(buf, sizeof(buf)), input_path
        );
        exit(-1);
    }

    // get size of input file (used to compute fraction done)
    //
    file_size(input_path, fsize);

    boinc_resolve_filename(OUTPUT_FILENAME, output_path, sizeof(output_path));

    // See if there's a valid checkpoint file.
    // If so seek input file and truncate output file
    //
    boinc_resolve_filename(CHECKPOINT_FILE, chkpt_path, sizeof(chkpt_path));
    state = boinc_fopen(chkpt_path, "r");
    if (state) {
        n = fscanf(state, "%d", &nchars);
        fclose(state);
    }
    if (state && n==1) {
        fseek(infile, nchars, SEEK_SET);
        boinc_truncate(output_path, nchars);
        retval = out.open(output_path, "ab");
    } else {
        retval = out.open(output_path, "wb");
    }
    if (retval) {
        fprintf(stderr, "%s APP: encrypt output open failed:\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
        fprintf(stderr, "%s resolved name %s, retval %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), output_path, retval
        );
        perror("open");
        exit(1);
    }

for(i=0; ;i++){
c = fgetc(infile);
if(c==EOF) break;
sentence[i]=c;
}
sentence[i]='\0';
p=43;
q=3;
pq=p*q;
//e=19 d=31
strcpy(sentence,rsaencrypt(sentence,19,pq));

    for (i=0;sentence[i]!='\0'; i++) {
	c=sentence[i];
        out._putchar(c);
        nchars++;
        if (run_slow) {
            boinc_sleep(1.);
        }

        if (early_exit && i>30) {
            exit(-10);
        }

        if (early_crash && i>30) {
            boinc_crash();
        }
        if (early_sleep && i>30) {
            boinc_disable_timer_thread = true;
            while (1) boinc_sleep(1);
        }

        if (boinc_time_to_checkpoint()) {
            retval = do_checkpoint(out, nchars);
            if (retval) {
                fprintf(stderr, "%s APP: encrypt checkpoint failed %d\n",
                    boinc_msg_prefix(buf, sizeof(buf)), retval
                );
                exit(retval);
            }
            boinc_checkpoint_completed();
        }

        fd = nchars/fsize;
        if (cpu_time) fd /= 2;
        boinc_fraction_done(fd);
    }

    retval = out.flush();
    if (retval) {
        fprintf(stderr, "%s APP: encrypt flush failed %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        exit(1);
    }

    boinc_fraction_done(1);
    boinc_finish(0);
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif

const char *BOINC_RCSID_33ac47a071 = "$Id$";

