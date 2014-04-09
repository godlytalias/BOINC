// Copyright (c) 2014 Godly T.Alias
//
// This is a free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// - Runs as a daemon, and creates an unbounded supply of work.
//   It attempts to maintain a "cushion" of 100 unsent job instances.
//   (your app may not work this way; e.g. you might create work in batches)
// - Creates work for the application "encrypt".
// - Creates a new input file for each job;
//   the file (and the workunit names) contain a timestamp
//   and sequence number, so they're unique.

#include <sys/param.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

 char cCurrentPath[FILENAME_MAX];

int n1,n2,start_time=0;

#define CUSHION 100
    // maintain at least this many unsent results
#define REPLICATION_FACTOR  1

const char* app_name = "graphiso";
const char* in_template_file = "graphiso_in";
const char* out_template_file = "graphiso_out";

char* in_template;
DB_APP app;
char graphs[256];

// create one new job
//
int make_job(int node,int sub_node) {
    DB_WORKUNIT wu;
    char name[256], path[MAXPATHLEN];
    const char* infiles[2];
    int retval;

    // make a unique name (for the job and its input file)
    //
    sprintf(name, "%s_%d_%d_%d", app_name,start_time, node,sub_node);
    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    retval = config.download_path(name, path);
    if (retval) return retval;
    FILE* f = fopen(path, "w");
    if (!f) return ERR_FOPEN;
    //no:of vertices  node_to_work
    fprintf(f,"%d %d %d\n",n1,node,sub_node);
    fclose(f);

    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    strcpy(wu.name, name);
    wu.rsc_fpops_est = n1*1e10;
    wu.rsc_fpops_bound = 1e24;
    wu.rsc_memory_bound = 1e8;
    wu.rsc_disk_bound = 1e8;
    wu.delay_bound = 30*n1;
    wu.min_quorum = REPLICATION_FACTOR;
    wu.target_nresults = REPLICATION_FACTOR;
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;
    infiles[0] = name;
    infiles[1] = graphs;

    // Register the job with BOINC
    //
    sprintf(path, "templates/%s", out_template_file);
    return create_work(
        wu,
        in_template,
        path,
        config.project_path(path),
        infiles,
        2,
        config
    );
}

void main_loop() {
    int retval;
    int node=0,n;
    while (node<n1) {
        check_stop_daemons();
	retval = count_unsent_results(n, 0);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "count_unsent_jobs() failed: %s\n", boincerror(retval)
            );
            exit(retval);
        }
        if (n > CUSHION) {
            daemon_sleep(10);
        } else {

          //  int njobs =  (CUSHION-n)/REPLICATION_FACTOR;
            log_messages.printf(MSG_DEBUG,
                "Making job for %d\n", node
            );
      //      for (int i=0; i<njobs; i++) {
      	      for (int i=0; i<n1; i+=999)
      	      {
                retval = make_job(node,i);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "can't make job: %s\n", boincerror(retval)
                    );
                    exit(retval);
                }
      	      }
   //         }
            // Now sleep for a few seconds to let the transitioner
            // create instances for the jobs we just created.
            // Otherwise we could end up creating an excess of jobs.
            double now = dtime();
            while (1) {
                daemon_sleep(5);
                double x;
                retval = min_transition_time(x);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "min_transition_time failed: %s\n", boincerror(retval)
                    );
                    exit(retval);
                }
                if (x > now) break;
            }
        }
        node++;
    }
}

void usage(char *name) {
    fprintf(stderr, "This is an example BOINC work generator.\n"
        "This work generator has the following properties\n"
        "(you may need to change some or all of these):\n"
        "  It attempts to maintain a \"cushion\" of 100 unsent job instances.\n"
        "  (your app may not work this way; e.g. you might create work in batches)\n"
        "- Creates work for the application \"example_app\".\n"
        "- Creates a new input file for each job;\n"
        "  the file (and the workunit names) contain a timestamp\n"
        "  and sequence number, so that they're unique.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ --app X                Application name (default: example_app)\n"
        "  [ --in_template_file     Input template (default: example_app_in)\n"
        "  [ --out_template_file    Output template (default: example_app_out)\n"
        "  [ -d X ]                 Sets debug level to X.\n"
        "  [ -h | --help ]          Shows this help text.\n"
        "  [ -v | --version ]       Shows version information.\n",
        name
    );
}


void get_graphs()
{
     int mode=0,temp;
     char path[MAXPATHLEN];
     char ch=' ';
     sprintf(graphs,"%s_graphs",app_name);
    config.download_path(graphs, path);
     FILE *f = fopen("g1.txt","r");
     FILE *write = fopen(path,"w");
      n1=0;n2=0;
         //checking the no: of nodes in the graph 1
	while(ch!='\n')
         {
         ch = fgetc(f);
          if(ch>=48 && ch<=57 && mode==0)
           {
             mode=1;
               n1++;
            }
             else if(ch<48 || ch>57)
              mode=0;
          }
          
          //initializing graph 1 and inputing values
         fseek(f,0,SEEK_SET);
         for(int i=0;i<n1;i++){
         for(int j=0;j<n1;j++){
         fscanf(f,"%d",&temp);
         fprintf(write,"%d ",temp);         
         }
         fprintf(write,"\n");}
         
         fclose(f);
         
         ch=' ';
mode=0;
f = fopen("g2.txt","r");
//reading the adjacent matrix of Graph 2
//first checking the no: of elements in a row
while(ch!='\n')
{
ch = fgetc(f);
 if(ch>=48 && ch<=57 && mode==0)
 {
  mode=1;
  n2++;
 }
 else if(ch==' ')
  mode=0;
}
         fseek(f,0,SEEK_SET);
         for(int i=0;i<n2;i++){
         for(int j=0;j<n2;j++){
         fscanf(f,"%d",&temp);
         fprintf(write,"%d ",temp);
         }
         fprintf(write,"\n");}
         fclose(f); 
         fclose(write);
     }

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];
 if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
     {
     return 0;
     }

cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (!strcmp(argv[i], "--app")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "--in_template_file")) {
            in_template_file = argv[++i];
        } else if (!strcmp(argv[i], "--out_template_file")) {
            out_template_file = argv[++i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    sprintf(buf, "templates/%s", in_template_file);
    if (read_file_malloc(config.project_path(buf), in_template)) {
        log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
        exit(1);
    }

    start_time = time(0);

    log_messages.printf(MSG_NORMAL, "Starting\n");

    get_graphs();
    if(n1==n2)
       main_loop();
    else{
    FILE *result = fopen("result.txt","w");
    fprintf(result,"NOT ISOMORPHIC");
    fclose(result);
    }
    //deleting memory allocated for arrays
}
