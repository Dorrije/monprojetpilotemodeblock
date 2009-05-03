#include <stdio.h>
#include <stdlib.h> 
#include <time.h> 



void Lecture_Fichier_Partielle(char *nom, int taille)
{

	FILE *f;
	char ligne[255]="";
	char caractereActuel;
	int i;
	
	if ((f=fopen(nom,"r"))==0)
	{
		fprintf(stderr,"erreur d'ouverture du fichier\n");
		exit(-1);
	}
	
	for (i=0;i<taille;i++)
        { 
        caractereActuel = fgetc(f); 
        printf("%c", caractereActuel);

         }
        printf("\n");
    	
	fclose(f);	
	
}




void Lecture_Fichier(char *nom)
{

	FILE *f;
	char ligne[255]="";
	
	if ((f=fopen(nom,"r"))==0)
	{
		fprintf(stderr,"erreur d'ouverture du fichier\n");
		exit(-1);
	}
	
        fgets (ligne,255,f);
	printf ("%s",ligne);

        printf("\n");
    	
	fclose(f);	
	
}



int Ecriture_Fichier(char* nom, char* chaine)
{
	FILE *f;
	int taille=0;
	taille = strlen(chaine)-1;

	if ((f=fopen(nom,"w"))==0)
	{
		fprintf(stderr,"erreur d'ouverture du fichier\n");
		exit(-1);
	}
             
	fprintf(f,"%s",chaine);

	fclose(f);
	return taille;
}




int main ( int argc, char* argv[] )
{
    char file[15];
    char nomdevice[20]="( /dev/mondevice )";
    char chaine[255]="";
   // chaine = (char*) malloc (sizeof(char*));
   
   int fin;
   fin = 0;
   
   int taille=0;
   
   printf("\n-->Donnez le nom du fichier  a lire ou a ecrire %s:\n",nomdevice);
            gets(file);
   while(!fin)
   {
      int c;
      
      /* affichage menu */
      printf("\n/***************** Menu:*********************/\n");
      printf("   1.Lecture du fichier \n"
             "   2.Ecriture du fichier \n"
             "   3.Quitter \n"
            );
      printf("\n Taper 1,2 ou 3 pour faire votre choix!"); 
 
      c = getchar();
 
      /* suppression des caracteres dans stdin */
      if(c != '\n' && c != EOF)
      {
         int d;
         while((d = getchar()) != '\n' && d != EOF);
      }
 
      switch(c)
      {
         case '1':
            printf("\n -->1. Lecture du fichier: \n");
            if (taille==0)
            Lecture_Fichier(file);
            else
            Lecture_Fichier_Partielle(file,taille);
            break;
 
         case '2':
            printf("\n --> 2. Ecriture du fichier ");
             printf("-->Donnez la chaine a ecrire : ");
             fgets(chaine, sizeof chaine, stdin);
             //printf("Chaine: %s\n", chaine);
             taille=Ecriture_Fichier(file,chaine);
            // printf("taille ==== %d\n",taille);   
            // Lecture_Fichier(file);
             break;
            
           case '3':
            fin = 1;
            break;
 
         default:
            printf("Choix errone\n");
      }
   }

   
   
   /***************Test Lecture************************/
    
    
    /************Test Ecriture ***********************/
   
   
   return 0;}
   
   
   
   

    

