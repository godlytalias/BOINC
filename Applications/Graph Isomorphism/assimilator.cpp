// Copyright (c) 2014 Godly T.Alias
//
// This is a free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.


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
char buf[512];
const char* app_name = "graphiso";

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
    char res[50];
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
	FILE *write;
	fscanf(read,"%s",res);
	if(strncmp(res,"ISOMORPHIC",10)==0)
	{
	write = fopen("result.txt","a");
        while(!feof(read))
        {
         fscanf(read,"%s",res);
         fprintf(write,"%s\n",res);
                          }
                          fprintf(write,"\n-----------------\n\n");
                                       }
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
