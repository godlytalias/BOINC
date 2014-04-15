// Copyright (c) 2014 Godly T.Alias
//
// This is a free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.


#include <stdio.h>
#include <vector>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#endif

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"

#define INPUT_FILENAME "in"
#define INPUT_GRAPH "graph"
#define OUTPUT_FILENAME "out"

bool run_slow = false;
bool early_exit = false;
bool early_crash = false;
bool early_sleep = false;
double cpu_time=20,comp_result;

const int MAX_PERMUTATIONS=1000;
const int CLIENT_LIMIT = 999;

using std::vector;
using namespace std;

#ifdef _WIN32
typedef HANDLE THREAD_ID;
typedef UINT (__stdcall *THREAD_FUNC)(void*);
#else
typedef void* (*THREAD_FUNC)(void*);
typedef pthread_t THREAD_ID;
#endif
#define THREAD_ID_NULL  0

struct THREAD {
    THREAD_ID id;
 int graph_id;
 float *g;
 int n;
 int initstate;

    THREAD(THREAD_FUNC func, int gid, float *graph,int no, int state) {
        char buf[256];

        graph_id = gid;
	g = graph;
	n = no;
	initstate = state;

#ifdef _WIN32
        id = (HANDLE) _beginthreadex(
            NULL,
            16384,
            func,
            this,
            0,
            NULL
        );
        if (!id) {
            fprintf(stderr, "%s Can't start thread\n",
                boinc_msg_prefix(buf, sizeof(buf))
            );
            exit(1);
        }
#else
        int retval;
        retval = pthread_create(&id, 0, func, (void*)this);
	if (retval) {
            fprintf(stderr, "%s Can't start thread\n",
                boinc_msg_prefix(buf, sizeof(buf))
            );
            exit(1);
        }
#endif
    }
};

struct THREAD_SET {
    vector<THREAD*> threads;
    bool all_done() {
        for (unsigned int i=0; i<threads.size(); i++) {
            if (threads[i]->id != THREAD_ID_NULL) return false;
        }
        return true;
    }};


int n1,n2,perm;
struct mapping
{
int map_ver;
float state;
int classid; };


float *g1,*g2;
int node,w_node,sub_node;
int tmp_count;
mapping *map_graph;

void max_heapify(float *a,mapping *pos, int i, int n)
{
    int j, temps;
    float temp;
    temps = pos[i].map_ver;
    temp = a[temps];
    j = 2*i;
    while (j <= n)
    {
        if (j < n && a[pos[j+1].map_ver] > a[pos[j].map_ver])
            j = j+1;
        if (temp > a[pos[j].map_ver])
            break;
        else if (temp <= a[pos[j].map_ver])
        {
            pos[j/2].map_ver = pos[j].map_ver;
            j = 2*j;
        }
    }
    pos[j/2].map_ver = temps;
    return;
}

void heapsort(float *a,mapping *pos, int end)
{
    int i, temps;
    for (i = end; i >= 2; i--)
    {
        temps = pos[i].map_ver;
        pos[i].map_ver = pos[1].map_ver;
        pos[1].map_ver = temps;
        max_heapify(a,pos, 1, i - 1);
    }
}

void build_maxheap(float *a,mapping *pos, int end)
{
    int i;
    for(i = end/2; i >= 1; i--)
    {
        max_heapify(a,pos, i, end);
    }
}

bool adj_mat_map(float *a1, float *a2)
{
int i,j;
for(i=0;i<node;i++)
 for(j=0;j<node;j++)
  if(a1[map_graph[i].map_ver*node+map_graph[j].map_ver]!=a2[map_graph[1*node+i].map_ver*node+map_graph[1*node+j].map_ver])
   return false;
return true;
}


int isotest(int p2_init_node,float *a1,float *a2)
{
    char filename[40];
    sprintf(filename,"../graphiso/map_1_%d",p2_init_node);
    FILE *read2 = fopen(filename,"r");
while(!feof(read2)){
for(int j=0;j<node;j++)
 fscanf(read2,"%d",&map_graph[node+j].map_ver);

  if(adj_mat_map(a1,a2))
 { 
   fclose(read2);
   return 2; } }

fclose(read2);
 return 0;
}


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
void matrix_prod(float *res,float *m1,int c1,float *m2,int r2,int c2)
{
float y,t,c;
if(c1==r2){
 for(int j=0;j<c2;j++){
 res[j]=0;
 c=0.0;
  for(int k=0;k<c1;k++){
//kahan summation to avoid precision lose
  y=(m1[k]*m2[k*c2+j])-c;
  t=res[j]+y;
  c = (t-res[j])-y;
  res[j]=t;}
 }
}
}

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
void permute(int start,int end,FILE *file,int graph_id,mapping *map_g,bool flag)
{
// if(perm<MAX_PERMUTATIONS){
 int t_start=0,t_end=0;
 if(start==end && flag)
 {
 perm++;
 for(int i=0;i<node;i++)
  fprintf(file,"%d ",map_g[i].map_ver);
 fprintf(file,"\n");
 }
 else
 {
 for(int i=0;i<=(end-start);i++)
  {
   swap(&map_g[start],&map_g[start+i]);
   if((start==0 || (start>0 && map_g[start].classid!=map_g[start-1].classid) || i!=0)&& end<(node-1))
   {
   for(int j=end+1;j<(node-1);j++)
    if(map_g[j].classid==map_g[j+1].classid)
     { t_start=j; break; }
   if(t_start>0)
   for(int j=t_start+1;j<node-1;j++)
    if(map_g[j].classid!=map_g[j+1].classid) //no need to check for last element in the row as it will always be different
     { t_end = j; break; }
   if(t_start!=t_end){
     permute(t_start,t_end,file,graph_id,map_g,true);
     flag=false;}}
   permute(start+1,end,file,graph_id,map_g,flag);
   flag=true;
   swap(&map_g[start+i],&map_g[start]);
   }
 }
}//}

//calculates the probability propogation matrix for the initial state initstate
#ifdef _WIN32
UINT WINAPI prob_prop_matrix(void* p) {
#else
void* prob_prop_matrix(void* p) {
#endif
    char buf[256];
    THREAD* t = (THREAD*)p;
    FILE *write;

  int graph_id = t->graph_id;
  float *g = t->g;
  int n = t->n;
  int initstate = t->initstate;

  sprintf(buf,"../graphiso/map_%d_%d",graph_id,initstate);

write = fopen(buf,"r");
if(!write)
{
mapping *map_g = new mapping[n1];
   write = fopen(buf,"w");

	float *row_mat,*row_mat_copy;
	int ptr = initstate*n;
if(initstate<n){
	 for(int i=0;i<n;i++)
        {
        map_g[i].map_ver=i;
        map_g[i].state=-1.0;
        map_g[i].classid=0;
        }
	 row_mat = new float[n];
	 row_mat_copy = new float[n];
	 
bool flag=true;
int start,end,j,temp,classptr;
float temps;

//writes the initial state vector to the row_mat
istate_dibn_vec(row_mat,initstate,n);
classptr=1;
for(int i=0;flag && i<((2*n)-1);i++)
{
for(j=0;j<n;j++)
 row_mat_copy[j]=row_mat[j];
        j=1;
        //this loop gives different class id to vertices with same class id but different state
        while(j<n)
        {
        if(map_g[j].classid==map_g[j-1].classid)
        {
         if(map_g[j].state!=map_g[j-1].state)
          {
            temp=map_g[j].classid;
          while(j<n && map_g[j].classid==temp){
            temps=map_g[j].state;
             while(j<n && map_g[j].state==temps && map_g[j].classid==temp){
              map_g[j].classid=classptr;
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
        while(j<n && map_g[end].classid==map_g[start].classid)
        {
          j++; end++;               
         }
        if(start<end-1){
           build_maxheap(row_mat,&map_g[-1+start],end-start);
           heapsort(row_mat,&map_g[-1+start],end-start); //subtracting 1 from array subscript for the padding for heap sort

        flag=true;
        } 
                              
        start=end;
        }
       
//writing state distribution vector to probability propogation matrix
for(j=0;j<n;j++)
 map_g[j].state=row_mat[map_g[j].map_ver];
 

 
//calculating the state distribution vector for string of next length
matrix_prod(row_mat,row_mat_copy,n,g,n,n);
}
 
delete [] row_mat;
delete [] row_mat_copy;

start=0;
while(start<n-1){
 if(map_g[start].classid==map_g[start+1].classid)
  break;
 start++; }
end=start+1;
while(end<n-1){
 if(map_g[end].classid!=map_g[end+1].classid)
  break;
 end++; }
 perm=0;
if(start<end && end<n)
 permute(start,end,write,graph_id,map_g,true);
else
{
for(int i=0;i<node;i++)
  fprintf(write,"%d ",map_g[i].map_ver);
 fprintf(write,"\n");   
}
fclose(write);
}
delete [] map_g;
}
else
 fclose(write);
t->id=THREAD_ID_NULL;
#ifdef _WIN32
    return 0;
#endif
}

//returns the degree of a vertix
int degree(float *m,int row,int n)
{
int deg=0;
int base_ptr=row*n;
for(int i=0;i<n;i++){
deg+=(int)m[base_ptr+i];}
return deg;
}

//computing the probability distribution matrices
void prob_dibn(float *m,int n)
{
int deg;
for(int i=0;i<n;i++){
 deg = degree(m,i,n);
 for(int j=0;j<n;j++)
 {
 m[i*n+j]/=deg;
 }
}
}

void get_graphs(FILE *f)
{
     int mode=0;
     char ch=' ';
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
          node=n1;
	  n2=n1;
          //initializing graph 1 and inputing values
         g1 = new float[n1*n1];
         fseek(f,0,SEEK_SET);
         for(int i=0;i<node;i++)
         for(int j=0;j<node;j++)
         fscanf(f,"%f ",&g1[i*n1+j]);

         g2 = new float[node*node];
         for(int i=0;i<n2;i++)
         for(int j=0;j<n2;j++)
         fscanf(f,"%f ",&g2[i*n2+j]);
                  
         //computing probability distribution matrices of both graphs
prob_dibn(g1,n1); //g1 is converted to the probability distribution matrix of graph 1
prob_dibn(g2,n2); //g2 is converted to the probability distribution matrix of graph 2          
     }


int main(int argc,char **argv) {

#if defined(_WIN32)
    _mkdir("../graphiso");
#elif defined(__linux__)
    mkdir("../graphiso", 0777);
// #else more?
#endif

    int retval,iso=0,limit;
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
        boinc_fraction_done(0);

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
        boinc_fraction_done(0.01);
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
        boinc_fraction_done(0.02);

get_graphs(infile);

fclose(infile);
        boinc_fraction_done(0.05);

  THREAD_SET thread_set;
	  thread_set.threads.push_back(new THREAD(prob_prop_matrix,0,g1,node,w_node));

        boinc_fraction_done(0.01);

limit = CLIENT_LIMIT + sub_node;

  for(pj=sub_node;(pj<limit)&&(pj<node);pj++)
	  thread_set.threads.push_back(new THREAD(prob_prop_matrix,1,g2,node,pj));
        boinc_fraction_done(0.2);

	while (1){
        if (thread_set.all_done()) break;
	boinc_sleep(1.0);}
        boinc_fraction_done(0.45);

  map_graph = new mapping[2*n1];
	iso=0;
  
    sprintf(filename,"../graphiso/map_0_%d",w_node);
    FILE *read1 = fopen(filename,"r");
 while(!feof(read1)&&(iso!=2)){
  for(int j=0;j<node;j++)
   fscanf(read1,"%d",&map_graph[j].map_ver);

   for(pj=sub_node;(pj<limit)&&(pj<node)&&(iso!=2);pj++){
    iso = isotest(pj,g1,g2);
    fd = ((pj-sub_node)/node)*0.5;    
        boinc_fraction_done(0.45+fd);}

  }

    
if(iso==2)
{
result=new char[node*10];
strcpy(result,"ISOMORPHIC\n");
for(long i=0;i<node;i++){
sprintf(buf,"%d->%d\n",map_graph[i].map_ver,map_graph[node+i].map_ver);
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

delete [] g1;
delete [] g2;
delete [] map_graph;
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
