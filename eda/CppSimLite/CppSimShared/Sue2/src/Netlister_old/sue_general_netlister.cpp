/**************************************************************************
 * FILE : sue_general_netlister.cpp
 * 
 * AUTHOR : Mike Perrott
 * 
 * DESCRIPTION : creates Hspice and CppSim netlists from SUE files.
 ************************************************************************/

/**************************************************************************
Copyright (c) 2004 Michael H Perrott
All rights reserved.


Permission is hereby granted, without written agreement and without
license or royalty fees, to use, copy, modify, and distribute this
software and its documentation for any purpose, provided that the
above copyright notice and the following two paragraphs appear in
all copies of this software.

IN NO EVENT SHALL MICHAEL H PERROTT BE LIABLE TO ANY PARTY FOR DIRECT,
INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF
THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF MICHAEL H
PERROTT HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

MICHAEL H PERROTT SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THE SOFTWARE PROVIDED
HEREUNDER IS ON AN "AS IS" BASIS, AND MICHAEL H PERROTT HAS NO
OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
MODIFICATIONS.
**************************************************************************/

#include "sue2_common_code.h"

main(int argc,char *argv[])
{
char top_sue_module_name[MAX_CHAR_LENGTH];
char netlist_filename[MAX_CHAR_LENGTH];
PARAMETER *param_list;
MODULE *first_module, *top_module;
FILE *outfile;
int i, print_top_subckt_flag;
char simulator_name[50];

strcpy(simulator_name,"NULL");
for (i = 0; argv[0][i] != '\0'; i++)
   {
   if (strncmp(&argv[0][i],"sue_spice_netlister",19) == 0)
     {
      strcpy(simulator_name,"hspice");
      break;
     }
   else if (strncmp(&argv[0][i],"sue_cppsim_netlister",20) == 0)
     {
      strcpy(simulator_name,"cppsim");
      break;
     }
   }
if (strcmp(simulator_name,"NULL") == 0)
   {
   printf("error in 'sue_general_netlister':  undefined program name\n");
   printf("   -> should be 'sue_spice_netlister' or 'sue_cppsim_netlister'\n");
   printf("      in this case:  '%s'\n",argv[0]);
   exit(1);
   }

if (argc < 3)
  {
   printf("error: need at least two arguments!\n");
   if (strcmp(simulator_name,"hspice") == 0)
       printf("  sue_spice_netlister sue_top_module sue_lib_file [outfile] [no_top]\n");
   else
      printf("  sue_cppsim_netlister sue_top_module sue_lib_file [outfile] [no_top]\n");
   exit(1);
  }

strcpy(top_sue_module_name,argv[1]);
/****
for (i = 0; argv[1][i] != '\0'; i++)
    top_sue_module_name[i] = tolower(argv[1][i]);
top_sue_module_name[i] = '\0';
****/

print_top_subckt_flag = 1;

if (argc > 4)
   {
   sprintf(netlist_filename,"%s",argv[3]);
   print_top_subckt_flag = 0;
   }
else if (argc > 3)
   {
   sprintf(netlist_filename,"%s",argv[3]);
   }
else
   {
   if (strcmp(simulator_name,"hspice") == 0)
      sprintf(netlist_filename,"%s.sp",argv[1]);
   else if (strcmp(simulator_name,"cppsim") == 0)
      sprintf(netlist_filename,"netlist.cppsim",argv[1]);
   }

if ((outfile = fopen(netlist_filename,"w")) == NULL)
  {
   printf("error in 'sue_spice_netlister':  can't open file:  '%s'\n",netlist_filename);
   exit(1);
 }

param_list = init_parameter();

//// extract modules
first_module = extract_sue_modules(argv[2],param_list);
// print_keyinfo_modules(first_module); exit(1);
warn_about_duplicate_modules(first_module);

//// process modules
connect_sue_instances_to_modules(first_module);
//print_keyinfo_modules(first_module);

first_module = trim_unused_modules(first_module,top_sue_module_name,
                                   simulator_name);
account_for_orient(first_module);

connect_wires_to_terminals_and_labels(first_module);
connect_wires_to_themselves(first_module);
//print_wires(first_module->next);
connect_instance_nodes(first_module);
trim_node_naming_instances_from_modules(first_module);
first_module = trim_node_naming_modules(first_module);
fill_in_missing_module_external_nodenames(first_module);
// prefix_x_to_instance_names(first_module);
// print_keyinfo_modules(first_module); exit(1);


top_module = determine_module_order(first_module);
top_module->top_module_flag = 1;
first_module = sort_modules(first_module);

if (strcmp(simulator_name,"hspice") == 0)
  {
   print_hspice_netlist(outfile,first_module,print_top_subckt_flag);
   printf("Hspice netlisting of cell '%s' completed with no errors\n",
          top_sue_module_name);
  }
 else
   {
   print_cppsim_netlist(outfile,first_module,print_top_subckt_flag);
   printf("CppSim netlisting of cell '%s' completed with no errors\n",
          top_sue_module_name);
   }
fclose(outfile);
}

