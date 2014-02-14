// Modified by Godly T.Alias

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California


#include <vector>
#include <string>
#include <cstdlib>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"

using std::vector;
using std::string;
const char* app_name = "encrypt";
char sen[1025];
char wuname[100],sno[100];
int sen_no;

int write_error(char* p) {
    static FILE* f = 0;
    if (!f) {
        f = fopen(config.project_path("sample_results/errors"), "a");
        if (!f) return ERR_FOPEN;
    }
    fprintf(f, "%s", p);
    fflush(f);
    return 0;
}

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    int retval;
    char buf[1024];
    unsigned int i;
    retval = boinc_mkdir(config.project_path("sample_results"));
    if (retval) return retval;

    if (wu.canonical_resultid) {
        vector<OUTPUT_FILE_INFO> output_files;
        const char *copy_path;
        get_output_file_infos(canonical_result, output_files);
        unsigned int n = output_files.size();
        bool file_copied = false;
        for (i=0; i<n; i++) {
            OUTPUT_FILE_INFO& fi = output_files[i];
            if (n==1) {
	FILE *read = fopen(fi.path.c_str(),"r");
	FILE *write = fopen("encrypted.txt","a");
		strcpy(wuname,wu.name);
		sscanf(wuname,"encrypt_%d",&sen_no);
		sprintf(sno,"\n|^%d^|\n ",sen_no);	
		fputs(sno,write);

		while(fgets(sen,1025,read)!=NULL)
		{
		fputs(sen,write);
		fputs("\n",write);
		}
	fputs("|||end|||",write);
	fclose(write);
	fclose(read);
                copy_path = config.project_path("sample_results/%s", wu.name);
            } else {
                copy_path = config.project_path("sample_results/%s_%d", wu.name, i);
            }
            retval = boinc_copy(fi.path.c_str() , copy_path);
            if (!retval) {
                file_copied = true;
            }
        }
        if (!file_copied) {
            copy_path = config.project_path(
                "sample_results/%s_%s", wu.name, "no_output_files"
            );
            FILE* f = fopen(copy_path, "w");
            fclose(f);
        }
    } else {
        sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
        return write_error(buf);
    }
    return 0;
}
