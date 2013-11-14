//  Writen by Kuren Charles <kuren@kuren.org>
//  1/22/2000
//  Ported to os/2 and updated by Richard Hasty
//  Jan 16 2005

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

int  result;
char pp=97;
int   i=0,j=0,k=0, jj=0, kk=0, mm=0, ll=0, nn=0;
int   exit_whileloop=0,
      temp2=0;
int   exif_header[12];
int   exif_data[100000];
int   length,
      place_in_file=0,
      is_motorola=0,
      Offset_to_first_IFD=8,
      ExifSubIFDOffset=0,
      IFD_Offset=0,
      number_of_directory_entry_contains_in_this_IFD=0,
      diag=1;
int   tag_number,
      data_format,
      number_of_components,
      offset_to_data_value,
      next_IFD_exists=0;
char  temp_char=0;
char  system_call[1024];
FILE *JPEG_file;
short int appendfilename;

int PlanarConfiguration;
char file_date[30];
char file_date2[20];
char file_path[800];
char file_name[256];


void find_start_of_image();
void examine_header();
void read_exif_data();
void discover_byte_order();
void calculate_offset();
void calculate_number_of_tags();
void process_IFD_entry();
void calculate_number_of_tags_in_SUB_IFD();
void process_SUB_IFD_entry();
void process_tag_number();
void check_for_another_IFD();
void seperate_path(char *);
void parse_filedate(void);
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
   if (argc >= 2)
      for (k=1;k<argc;k++)
      {
         if (argv[k][0]=='/')
         {
            switch (argv[k][1])
            {
               case 'a':
               case 'A':
                  appendfilename=1;
                  break;
               default:
                  printf("Invalid Switch  %s please read the docs.\n", argv[k]);
                  break;
            } // switch
        } // if
        else
        {
            file_date[0]=0;
            file_date2[0]=0;
            system_call[0]=0;
            file_path[0]=0;
            seperate_path(argv[k]);
            JPEG_file=fopen(argv[k], "rb");
            if (JPEG_file != NULL)
            {
               find_start_of_image();
               examine_header();
               read_exif_data();
               discover_byte_order();
               calculate_offset();
               do
               {
                  calculate_number_of_tags();
                  for (i=0;i<number_of_directory_entry_contains_in_this_IFD;i++)
                     process_IFD_entry();
                     check_for_another_IFD();
               }while (next_IFD_exists);
               calculate_number_of_tags_in_SUB_IFD();
               for (i=0;i<number_of_directory_entry_contains_in_this_IFD;i++)
                  process_SUB_IFD_entry();
               fclose(JPEG_file);
               parse_filedate();
               strcat(system_call,file_path);
               strcat(system_call,file_date);
               if (appendfilename==1) 
               {
                    strcat(system_call, "_");
                    strcat(system_call, file_name);
               }
               else
                    strcat(system_call, ".jpg");
               result = rename( argv[k], system_call);
               if( result == 0 )
                  printf( "File '%s' renamed to '%s'\n", argv[k],system_call);
               else
               {
                  printf( "Could not rename '%s' to '%s'\n", argv[k],system_call);
                  strcpy(file_date2,file_date);
                  pp=97;
                  do
                  {
                     result=strlen(file_date2);
                     file_date2[result]=pp;
                     file_date2[result+1]=0;
                     strcpy(system_call,file_path);
                     strcat(system_call,file_date2);
                     if (appendfilename==1) 
                        strcat(system_call, file_name);
                     else
                        strcat(system_call,".jpg");
                     result = rename( argv[k], system_call);
                     if(result == 0)
                     {
                        printf( "File '%s' renamed to '%s'\n", argv[k],system_call);
                        pp=123;
                     }
                        else
                           printf( "Could not rename '%s' to '%s'\n", argv[k],system_call);
                           pp++;
                  }while (pp<122);

            }
          }
         //scanf("Enter a charater to continue\n%c",temp_char);
      }
   }
   else
   {
      printf("EXIF jpeg re-namer, version 0.20000116 by Kuren <kuren@kuren.org>.\n");
      printf("Ported to os/2 and updated by Richard Hasty.\n");
      printf("Tell this program which file(s) you want to rename (by copying a new file).\n");
      printf("usage:\nexifren [/a] DSCN0004.jpg\n");
      return 1;
   }
   return 0;
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void find_start_of_image()
{
   // searching for JPEG header
   while (JPEG_file != NULL)
   {
      temp2=fgetc(JPEG_file);
      ++place_in_file;
      if (temp2==255)
      {
         temp2=fgetc(JPEG_file);
         ++place_in_file;
         if (temp2==216)
         {
            // JPEG SOI marker found at
            place_in_file-=2;
            fseek(JPEG_file,place_in_file,0);
            break;
         }
      }
   }
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void examine_header()
{
   //examine header
   for(i=0;i<12;i++)
   if ((exif_header[i]=fgetc(JPEG_file))==EOF)
   {
      printf("End of file found prematurely.\n");
      printf("ERROR");
      exit(1);
   }
   // reading TIFF header
   // check for APP1 Marker used by Exif
   if (exif_header[2]!=255 && exif_header[3]!=225)
      printf("This file is not an Exif file.\n");
   // APP1 marker found

   // calculate length of APP1 data
   length=(exif_header[4]*256)+(exif_header[5]);
   // APP1 data length = length
   if (length >= 100000)
   {
      printf("The data is too large for this program to cope with\n");
      printf("ERROR");
      exit(1);
   }
   //check to see if "Exif" tag is there
   if (!(exif_header[6]=='E' && exif_header[7]=='x' && exif_header[8]=='i' && exif_header[9]=='f' && exif_header[10]==0 && exif_header[11]==0))
   {
      printf("\"EXIF\" was not found as expected.\n");
      printf("ERROR");
      exit(1);
   }
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void read_exif_data()
{
   fseek(JPEG_file,-8,SEEK_CUR);
   for (i=0;i<length;++i)
   {
      if ((exif_data[i]=fgetc(JPEG_file))==EOF)
      {
         printf("End of file found unexpectedly while reading APP1 data.\n");
         printf("ERROR");
         exit(1);
      }
   }			
}
void discover_byte_order()
{
   if (exif_data[8]=='I' && exif_data[9]=='I')
   {
      is_motorola=0;
      //printf("intel byte order\n");                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
   }
   if (exif_data[8]=='M' && exif_data[9]=='M')
   {
      is_motorola=1;
      //printf("motorola byte order\n");
   }
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void calculate_offset()
{
   // calculate offset
   if (is_motorola)
      Offset_to_first_IFD=exif_data[12]*256*256*256+
                          exif_data[13]*256*256+
                          exif_data[14]*256+
                          exif_data[15];
   else
      Offset_to_first_IFD=exif_data[15]*256*256*256+
                          exif_data[14]*256*256+
                          exif_data[13]*256+
                          exif_data[12];
   IFD_Offset=8+Offset_to_first_IFD;
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void calculate_number_of_tags()
{
   // number of directory entry contains in this IFD
   if (is_motorola)
      number_of_directory_entry_contains_in_this_IFD=exif_data[0+IFD_Offset]*256+
                                                     exif_data[1+IFD_Offset];
   else
      number_of_directory_entry_contains_in_this_IFD=exif_data[1+IFD_Offset]*256+
                                                     exif_data[0+IFD_Offset];
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void process_IFD_entry()
{
   if (is_motorola)
   {
      tag_number=exif_data[2+IFD_Offset+i*12]*256+
                 exif_data[3+IFD_Offset+i*12];
      data_format=exif_data[4+IFD_Offset+i*12]*256+
                  exif_data[5+IFD_Offset+i*12];
      number_of_components=exif_data[6+IFD_Offset+i*12]*256*256*256+
                           exif_data[7+IFD_Offset+i*12]*256*256+
                           exif_data[8+IFD_Offset+i*12]*256+
                           exif_data[9+IFD_Offset+i*12];
      offset_to_data_value=exif_data[10+IFD_Offset+i*12]*256*256*256+
                           exif_data[11+IFD_Offset+i*12]*256*256+
                           exif_data[12+IFD_Offset+i*12]*256+
                           exif_data[13+IFD_Offset+i*12];
   }
   else
   {
      tag_number=exif_data[3+IFD_Offset+i*12]*256+
                 exif_data[2+IFD_Offset+i*12];
      data_format=exif_data[5+IFD_Offset+i*12]*256+
                  exif_data[4+IFD_Offset+i*12];
      number_of_components=exif_data[9+IFD_Offset+i*12]*256*256*256+
                           exif_data[8+IFD_Offset+i*12]*256*256+
                           exif_data[7+IFD_Offset+i*12]*256+
                           exif_data[6+IFD_Offset+i*12];
      offset_to_data_value=exif_data[13+IFD_Offset+i*12]*256*256*256+
                           exif_data[12+IFD_Offset+i*12]*256*256+
                           exif_data[11+IFD_Offset+i*12]*256+
                           exif_data[10+IFD_Offset+i*12];
   }
   process_tag_number();
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void calculate_number_of_tags_in_SUB_IFD()
{
   if (is_motorola)
      number_of_directory_entry_contains_in_this_IFD=exif_data[ExifSubIFDOffset]*256+exif_data[1+ExifSubIFDOffset];
   else
      number_of_directory_entry_contains_in_this_IFD=exif_data[1+ExifSubIFDOffset]*256+exif_data[ExifSubIFDOffset];
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void process_SUB_IFD_entry()
{
   IFD_Offset=ExifSubIFDOffset;
   if (is_motorola)
   {
      tag_number=exif_data[2+IFD_Offset+i*12]*256+
                 exif_data[3+IFD_Offset+i*12];
      data_format=exif_data[4+IFD_Offset+i*12]*256+
                  exif_data[5+IFD_Offset+i*12];
      number_of_components=exif_data[6+IFD_Offset+i*12]*256*256*256+
                           exif_data[7+IFD_Offset+i*12]*256*256+
                           exif_data[8+IFD_Offset+i*12]*256+
                           exif_data[9+IFD_Offset+i*12];
      offset_to_data_value=exif_data[10+IFD_Offset+i*12]*256*256*256+
                           exif_data[11+IFD_Offset+i*12]*256*256+
                           exif_data[12+IFD_Offset+i*12]*256+
                           exif_data[13+IFD_Offset+i*12];
   }
   else
   {
      tag_number=exif_data[3+IFD_Offset+i*12]*256+
                 exif_data[2+IFD_Offset+i*12];
      data_format=exif_data[5+IFD_Offset+i*12]*256+
                  exif_data[4+IFD_Offset+i*12];
      number_of_components=exif_data[9+IFD_Offset+i*12]*256*256*256+
                           exif_data[8+IFD_Offset+i*12]*256*256+
                           exif_data[7+IFD_Offset+i*12]*256+
                           exif_data[6+IFD_Offset+i*12];
      offset_to_data_value=exif_data[13+IFD_Offset+i*12]*256*256*256+
                           exif_data[12+IFD_Offset+i*12]*256*256+
                           exif_data[11+IFD_Offset+i*12]*256+
                           exif_data[10+IFD_Offset+i*12];
   }
   process_tag_number();
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void process_tag_number()
{
  switch(tag_number)
   {
      case   306: // printf("DateTime                     ascii string               20");	 break;

                  nn=0;
                  for (j=0;j<number_of_components-1;j++)
                     if (exif_data[8+offset_to_data_value+j]<='9' && exif_data[8+offset_to_data_value+j]>='0')
                        file_date[nn++]=exif_data[8+offset_to_data_value+j];

                  break;
      case 34665: // printf("ExifOffset                   unsigned long               1");	 break;
                  ExifSubIFDOffset=offset_to_data_value+8;
                  break;
   }
}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

void check_for_another_IFD()
{
   int next_IFD;
   if (is_motorola)
   {
      next_IFD=exif_data[IFD_Offset+number_of_directory_entry_contains_in_this_IFD*12+2]*256*256*256+
               exif_data[IFD_Offset+number_of_directory_entry_contains_in_this_IFD*12+3]*256*256+
               exif_data[IFD_Offset+number_of_directory_entry_contains_in_this_IFD*12+4]*256+
               exif_data[IFD_Offset+number_of_directory_entry_contains_in_this_IFD*12+5];
   }
   else
   {
      next_IFD=exif_data[IFD_Offset+number_of_directory_entry_contains_in_this_IFD*12+5]*256*256*256+
               exif_data[IFD_Offset+number_of_directory_entry_contains_in_this_IFD*12+4]*256*256+
               exif_data[IFD_Offset+number_of_directory_entry_contains_in_this_IFD*12+3]*256+
               exif_data[IFD_Offset+number_of_directory_entry_contains_in_this_IFD*12+2];
   }
   IFD_Offset=next_IFD+8;
   next_IFD_exists=next_IFD;

}
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
void seperate_path(char *fullname)
{
   int i=0;
   int end=0;
   while(fullname[i]!=0)
   {
      if (fullname[i]==92)
         end=i;
      i++;
   }
    
   if (end!=0)
   {
        strncpy(file_path, fullname, end+1);
        file_path[end+1]=0;
        strncpy(file_name, fullname+end+1, strlen(fullname+end+1));
    }
   else
   {
      strcpy(file_name, fullname);
      file_path[0]=0;
    }
}

void parse_filedate(void)
{
    char year[5];
    char month[3];
    char day[3];
    char hour[3];
    char minute[3];
    char second[3];

    strncpy(year, file_date, 4);
    strncpy(month, file_date+4, 2);
    strncpy(day, file_date+6, 2);
    strncpy(hour, file_date+8, 2);
    strncpy(minute, file_date+10, 2);
    strncpy(second, file_date+12, 2);
    year[4]=0;
    month[2]=0;
    day[2]=0;
    hour[2]=0;
    minute[2]=0;
    second[2]=0;
    sprintf(file_date,"%s-%s-%s_%sh%sm%s", year, month, day, hour, minute, second);
}
