// Copyright (c) 2014 Godly T.Alias
//
// This is a free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.


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
#include <sys/stat.h>
#endif

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"


using std::string;

#define INPUT_FILENAME "in"
#define INPUT_GRAPH "graph"
#define OUTPUT_FILENAME "out"

bool run_slow = false;
bool early_exit = false;
bool early_crash = false;
bool early_sleep = false;
double cpu_time=20,comp_result;

const int MAX_PERMUTATIONS=1000;

int perm;
struct mapping
{
int map_ver;
float state;
int classid; };

mapping **map_g;
float **g1,**g2;
int node,w_node,sub_node;
int tmp_count;


//merge the partitions made by the mergesort
void merge(float *g,int s1,int e1,int s2,int e2,int graph_id)
{
int i,j,temp;
float a,b;
 if(s1<e2 && (e1+1)==s2)
 {
  i=s1;
  j=s2;
  while(i<=e1 && j<=e2)
  {
   a=g[map_g[graph_id][i].map_ver];
   b=g[map_g[graph_id][j].map_ver];
   if(a<b)
   i++;
   else if(a>=b)
    {
     temp = map_g[graph_id][j].map_ver;
     for(int k=j;k>i;k--)
     map_g[graph_id][k].map_ver=map_g[graph_id][k-1].map_ver;
     map_g[graph_id][i].map_ver=temp;
     j++;
     i++; e1++;
    }
  }
 }
}

//mergesort the matrix a and write the mapping to map_g[mat]
void mergesort(float *g,int start, int end,int graph_id)
{
if(start<(end-1))
 {
 mergesort(g,start,(start+end)/2,graph_id);
 mergesort(g,((start+end)/2)+1,end,graph_id);
 merge(g,start,(start+end)/2,((start+end)/2)+1,end,graph_id);
 }
else
 merge(g,start,start,end,end,graph_id);
 
}

bool adj_mat_map(float **a1, float **a2)
{
int i,j;
for(i=0;i<node;i++)
 for(j=0;j<node;j++)
  if(a1[map_g[0][i].map_ver][map_g[0][j].map_ver]!=a2[map_g[1][i].map_ver][map_g[1][j].map_ver])
   return false;
return true;
}


int isotest(int p1_init_node,int p2_init_node,float **a1,float **a2)
{
    char filename[40];
    sprintf(filename,"../graphiso/map_0_%d",p1_init_node);
    FILE *read1 = fopen(filename,"r");
    sprintf(filename,"../graphiso/map_1_%d",p2_init_node);
    FILE *read2 = fopen(filename,"r");
while(!feof(read1)){
for(int i=0;i<node;i++)
 fscanf(read1,"%d",&map_g[0][i].map_ver);

while(!feof(read2)){
for(int j=0;j<node;j++)
 fscanf(read2,"%d",&map_g[1][j].map_ver);

  if(adj_mat_map(a1,a2))
 { fclose(read1);
   fclose(read2);
   return 2; } }
      
 fseek(read2,0,SEEK_SET);
    }
fclose(read1);
fclose(read2);
 return 0;
}

float *row_mat,*row_mat_copy;
//returns the initial state distribution vector
void istate_dibn_vec(float* init_state, int i,int n)
{
for(int j=0;j<n;j++)
 if(j==i)
  init_state[j]=1.0;
 else
  init_state[j]=0.0;
}

//computes the product of matrices m1 & m2 and write the result in res matrix
void matrix_prod(float *res,float *m1,int c1,float **m2,int r2,int c2)
{
float y,t,c;
if(c1==r2){
 for(int j=0;j<c2;j++){
 res[j]=0;
 c=0.0;
  for(int k=0;k<c1;k++){
//kahan summation to avoid precision lose
  y=(m1[k]*m2[k][j])-c;
  t=res[j]+y;
  c = (t-res[j])-y;
  res[j]=t;}
 }
}}

//swaps the given parameters
void swap(mapping *a,mapping *b)
{
 mapping temp;
 temp=*a;
 *a=*b;
 *b=temp;     
}

//output all the possible mappings when 2 column vectors of 
//probability propogation matrix becomes equal
void permute(int start,int end,FILE *file,int graph_id,bool flag)
{
// if(perm<MAX_PERMUTATIONS){
 int t_start=0,t_end=0;
 if(start==end && flag)
 {
 perm++;
 for(int i=0;i<node;i++)
  fprintf(file,"%d ",map_g[graph_id][i].map_ver);
 fprintf(file,"\n");
 }
 else
 {
 for(int i=0;i<=(end-start);i++)
  {
   swap(&map_g[graph_id][start],&map_g[graph_id][start+i]);
   if((start==0 || (start>0 && map_g[graph_id][start].classid!=map_g[graph_id][start-1].classid) || i!=0)&& end<(node-1))
   {
   for(int j=end+1;j<(node-1);j++)
    if(map_g[graph_id][j].classid==map_g[graph_id][j+1].classid)
     { t_start=j; break; }
   if(t_start>0)
   for(int j=t_start+1;j<node-1;j++)
    if(map_g[graph_id][j].classid!=map_g[graph_id][j+1].classid) //no need to check for last element in the row as it will always be different
     { t_end = j; break; }
   if(t_start!=t_end){
     permute(t_start,t_end,file,graph_id,true);
     flag=false;}}
   permute(start+1,end,file,graph_id,flag);
   flag=true;
   swap(&map_g[graph_id][start+i],&map_g[graph_id][start]);
   }
 }
}//}

//calculates the probability propogation matrix for the initial state initstate
void prob_prop_matrix(int graph_id, float **g, int n, int initstate)
{
char file_name[40];
bool flag=true;
int start,end,j,temp,classptr;
float temps;
sprintf(file_name,"../graphiso/map_%d_%d",graph_id,initstate);
FILE *write = fopen(file_name,"w");
//row_mat holds the value of each state distribution vector
row_mat = new float[n];
row_mat_copy = new float[n];
//writes the initial state vector to the row_mat
istate_dibn_vec(row_mat,initstate,n);
classptr=1;
for(int i=0;flag && i<((2*n)-1);i++)
{
        j=1;
        //this loop gives different class id to vertices with same class id but different state
        while(j<n)
        {
        if(map_g[graph_id][j].classid==map_g[graph_id][j-1].classid)
        {
         if(map_g[graph_id][j].state!=map_g[graph_id][j-1].state)
          {
            temp=map_g[graph_id][j].classid;
          while(j<n && map_g[graph_id][j].classid==temp){
            temps=map_g[graph_id][j].state;
             while(j<n && map_g[graph_id][j].state==temps && map_g[graph_id][j].classid==temp){
              map_g[graph_id][j].classid=classptr;
              j++;
              }
              classptr++;
              }  
             }  
          else j++;
             }
         else j++;
                  }
                  
        start=0;
        j=0;
        flag=false;
        while(j<n)
        {
        end=start+1;
        j++;
        while(j<n && map_g[graph_id][end].classid==map_g[graph_id][start].classid)
        {
          j++; end++;               
         }
        if(start<end-1){
        mergesort(row_mat,start,end-1,graph_id);
        flag=true;}
                
        start=end;
        }
       
//writing state distribution vector to probability propogation matrix
for(j=0;j<n;j++){
 row_mat_copy[j]=row_mat[j];
 map_g[graph_id][j].state=row_mat[map_g[graph_id][j].map_ver];
 }
 
//calculating the state distribution vector for string of next length
matrix_prod(row_mat,row_mat_copy,n,g,n,n);
}

delete [] row_mat;
delete [] row_mat_copy;
start=0;
while(start<n-1){
 if(map_g[graph_id][start].classid==map_g[graph_id][start+1].classid)
  break;
 start++; }
end=start+1;
while(end<n-1){
 if(map_g[graph_id][end].classid!=map_g[graph_id][end+1].classid)
  break;
 end++; }
 perm=0;
if(start<end && end<n)
 permute(start,end,write,graph_id,true);
else
{
for(int i=0;i<node;i++)
  fprintf(write,"%d ",map_g[graph_id][i].map_ver);
 fprintf(write,"\n");   
}
fclose(write);
}

//returns the degree of a vertix
int degree(float **m,int row,int n)
{
int deg=0;
for(int i=0;i<n;i++)
deg+=(int)m[row][i];
return deg;
}

//computing the probability distribution matrices
void prob_dibn(float **m,int n)
{
int deg;
for(int i=0;i<n;i++){
 deg = degree(m,i,n);
 for(int j=0;j<n;j++)
 {
 m[i][j]/=deg;
 }
}
}

void get_graphs(FILE *f)
{
         
          //initializing graph 1 and inputing values
         g1 = new float*[node];
         for(int i=0;i<node;i++)
         g1[i]=new float[node];
         for(int i=0;i<node;i++)
         for(int j=0;j<node;j++)
         fscanf(f,"%f",&g1[i][j]);
         
         g2 = new float*[node];
         for(int i=0;i<node;i++)
         g2[i]=new float[node];
         for(int i=0;i<node;i++)
         for(int j=0;j<node;j++)
         fscanf(f,"%f",&g2[i][j]);
         
         //computing probability distribution matrices of both graphs
prob_dibn(g1,node); //g1 is converted to the probability distribution matrix of graph 1
prob_dibn(g2,node); //g2 is converted to the probability distribution matrix of graph 2          
     }


int main(int argc,char **argv) {

#if defined(_WIN32)
    _mkdir("../graphiso");
#elif defined(__linux__)
    mkdir("../graphiso", 0777);
// #else more?
#endif

    int retval,iso=0;
    int pj;
    double fd;
    char input_path[512], output_path[512], buf[256];
    MFILE out;
    FILE *infile;
char *result;
char filename[60];

   
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
    
    
    fscanf(infile,"%d %d %d",&node,&w_node,&sub_node);
    fclose(infile);

    boinc_resolve_filename(OUTPUT_FILENAME, output_path, sizeof(output_path));


        retval = out.open(output_path, "wb");

    if (retval) {
        fprintf(stderr, "%s APP: graphiso output open failed:\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
        fprintf(stderr, "%s resolved name %s, retval %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), output_path, retval
        );
        perror("open");
        exit(1);
    }



map_g = new mapping*[2];
for(int i=0;i<2;i++)
map_g[i]=new mapping[node];

 // open the input file for graphs(resolve logical name first)
    //
    boinc_resolve_filename(INPUT_GRAPH, input_path, sizeof(input_path));
    infile = boinc_fopen(input_path, "r");
    if (!infile) {
        fprintf(stderr,
            "%s Couldn't find input file for graph, resolved name %s.\n",
            boinc_msg_prefix(buf, sizeof(buf)), input_path
        );
        exit(-1);
    }

get_graphs(infile);

fclose(infile);

FILE *read;
  sprintf(filename,"../graphiso/map_%d_%d",0,w_node);  
  read=fopen(filename,"r");
  if(!read){
      for(int i=0;i<node;i++)
        {
        map_g[0][i].map_ver=i;
        map_g[0][i].state=-1.0;
        map_g[0][i].classid=0;
        }
	  prob_prop_matrix(0,g1,node,w_node);
   }
  else
	  fclose(read);

  for(pj=sub_node;(pj<node)&&(iso!=2);pj++)
  {
   sprintf(filename,"../graphiso/map_%d_%d",1,pj);
   read=fopen(filename,"r");
   if(!read){
       for(int i=0;i<node;i++)
        {
        map_g[1][i].map_ver=i;
        map_g[1][i].state=-1.0;
        map_g[1][i].classid=0;
        }
	   prob_prop_matrix(1,g2,node,pj);}
   else
	   fclose(read);

   iso = isotest(w_node,pj,g1,g2);

        fd = pj/node;
        boinc_fraction_done(fd);
  }

    
if(iso==2)
{
result=new char[node*10];
strcpy(result,"ISOMORPHIC\n");
for(long i=0;i<node;i++){
sprintf(buf,"%d->%d\n",map_g[0][i].map_ver,map_g[1][i].map_ver);
strcat(result,buf);
}
}
else
{
result=new char[20];
strcpy(result,"NOT ISOMORPHIC\n");   
}



    for (long i=0;result[i]!='\0'; i++) {
        out._putchar(result[i]);
    }
	delete [] result;
    retval = out.flush();
    if (retval) {
        fprintf(stderr, "%s APP: graphiso flush failed %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        exit(1);
    }

for(int i = 0; i < node; i++) {
    delete [] g1[i];
}
delete [] g1;
for(int i = 0; i < node; i++) {
    delete [] g2[i];
}
delete [] g2;
for(int i=0;i<=1;i++)  {
    delete [] map_g[i];
}
delete [] map_g;
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
