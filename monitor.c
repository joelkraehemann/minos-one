#include <sys/resource.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include <libxml/parser.h>
#include <libxml/xlink.h>
#include <libxml/xpath.h>
#include <libxml/valid.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>

#include <glib.h>

#define DEFAULT_ENCODING "UTF-8\0"

GList* lfsm_find_files(char *path);
GList* lfsm_list_previous(char *filename);

GList*
lfsm_find_files(char *path)
{
  DIR *directory;
  struct dirent *entry;
  GList *files;
  char *working_directory;

  files = NULL;

  /* scan */
  directory = opendir(path);
  chdir(path);

  working_directory = get_current_dir_name();
  printf("%s\n\0", working_directory);

  if(!strcmp("/usr/sources\0",
	     working_directory) ||
     !strcmp("/cdrom\0",
	     working_directory) ||
     !strcmp("/mnt\0",
	     working_directory) ||
     !strcmp("/dev\0",
	     working_directory) ||
     !strcmp("/run\0",
	     working_directory) ||
     !strcmp("/sys\0",
	     working_directory) ||
     !strcmp("/proc\0",
	     working_directory) ||
     !strcmp("/tools\0",
	     working_directory)){
    return(NULL);
  }

  while((entry = readdir(directory)) != NULL){
    switch(entry->d_type){
    case DT_REG:
      {
	files = g_list_prepend(files, g_strdup_printf("%s/%s\0",
						      working_directory,
						      entry->d_name));

        break;
      }
    case DT_DIR:
      {
        GList *child_files;

        if(strncmp(entry->d_name, ".\0", 2) != 0 && strncmp(entry->d_name, "..\0", 3) != 0){
          child_files = lfsm_find_files(entry->d_name);

	  if(child_files != NULL){
	    files = g_list_concat(files, child_files);
	  }

          chdir("..\0");
        }

        break;
      }
    case DT_UNKNOWN:
      {
        break;
      }
    }
  }

  closedir(directory);

  return(files);
}

GList*
lfsm_list_previous(char *filename)
{
  FILE *filenames;
  GList *list;
  char *str;

  filenames = fopen(filename,
		    "r+\0");
  
  list = NULL;
  
  while(!feof(filenames)){
    fscanf(filenames, "%ms\n\0", &str);

    if(list == NULL){
      list = g_list_alloc();
      list->data = str;
    }else{
      list = g_list_prepend(list,
			    str);
    }
  }

  fclose(filenames);

  list = g_list_reverse(list);

  return(list);
}

int
main(int argc, char **argv)
{
  xmlDoc *doc;
  FILE *filenames, *out;
  xmlNode *root_node;
  xmlNode *node, *child;
  GList *prev;
  GList *diff;
  GList *files, *files_start;
  GList *list;
  char *path;
  char *filename, *docname;
  char *package;
  gchar *str;

  char *working_directory;
  char *buffer;
  int size;

  const rlim_t kStackSize = 8192L * 1024L * 1024L;   // min stack size = 512 Mb
  struct rlimit rl;
  int result;

  if(argc != 5){
    printf("monitor usage: path filename docname package\n\0");

    return(-1);
  }

  result = getrlimit(RLIMIT_STACK, &rl);
  if(result == 0){
    if(rl.rlim_cur < kStackSize){
      rl.rlim_cur = kStackSize;
      result = setrlimit(RLIMIT_STACK, &rl);

      if(result != 0){
	fprintf(stderr, "setrlimit returned result = %d\n", result);
      }
    }
  }

  path = argv[1];
  filename = argv[2];
  docname = argv[3];
  package = argv[4];

  working_directory = get_current_dir_name();

  diff = NULL;

  /* read previous */
  prev = lfsm_list_previous(filename);

  /* find files */
  files_start = 
    files = lfsm_find_files(path);

  /* compare */
  while(files != NULL){
    list = prev;

    while(list != NULL){
      if(!strcmp(list->data,
		 files->data)){
	break;
      }

      list = list->next;
    }

    if(list == NULL){
      diff = g_list_prepend(diff,
			    files->data);
    }

    files = files->next;
  }

  /* restore */
  chdir(working_directory);

  /* save filenames */
  str = g_strdup_printf("%s/%s\0",
			working_directory,
			filename);

  filenames = fopen(str,
		    "a+\0");

  g_free(str);

  list = diff;

  while(list != NULL){
    fprintf(filenames,
	    "%s\n\0",
	    list->data);

    list = list->next;
  }

  fclose(filenames);

  /* save package */
  str = g_strdup_printf("%s/%s\0",
			working_directory,
			docname);

  doc = xmlReadFile(str, NULL, 0);
  out = fopen(str,
	      "w+\0");

  g_free(str);

  root_node = xmlDocGetRootElement(doc);

  node = xmlNewNode(NULL,
		    "package\0");
  xmlNewProp(node,
	     "name\0",
	     package);

  list = diff;

  while(list != NULL){
    child = xmlNewNode(NULL,
		       "filename\0");
    xmlNewProp(child,
	       "url\0",
	       list->data);
    xmlAddChild(node,
		child);

    list = list->next;
  }

  xmlAddChild(root_node,
	      node);

  xmlDocDumpFormatMemoryEnc(doc, &buffer, &size, DEFAULT_ENCODING, 1);

  fwrite(buffer, size, sizeof(xmlChar), out);
  fflush(out);
  fclose(out);

  /*free the document */
  xmlFreeDoc(doc);

  /*
   *Free the global variables that may
   *have been allocated by the parser.
   */
  xmlCleanupParser();

  /*
   * this is to debug memory for regression tests
   */
  xmlMemoryDump();

  return(0);
}
