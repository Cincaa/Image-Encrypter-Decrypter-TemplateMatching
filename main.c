#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include<math.h>

typedef struct {
    unsigned char b,g,r;
}RGB;

typedef struct {
    unsigned char x;
}HEADER;

typedef struct {
    int x,y;
}COORDONATE;

typedef struct {
    int x,y,cif;
    double ps;
}DETECTIE;
unsigned int XORSHIFT32(unsigned int st){

    unsigned int x = st;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	st = x;
	return x;
}

void Liniarizare( char *nume_fisier, RGB **v,HEADER **h, int *hei, int *wid)
{

    int height, width;
    FILE *fin;

    fin=fopen(nume_fisier,"rb");

    if(fin==NULL)
        printf("Incarcare esuata\n");
    else
        printf("Incarcare reusita\n");

    *h = (HEADER*)malloc(54 * sizeof(HEADER));

    fseek(fin, 18, SEEK_SET);
                                                ///se citesc dimansiuniile imaginii
    fread(&width, sizeof(unsigned int), 1, fin);
    fread(&height, sizeof(unsigned int), 1, fin);

    *v = (RGB*)malloc(height*width * sizeof(RGB));

    fseek(fin,0,SEEK_SET);

    for ( int i = 0; i < 54; i++) ///citim headerul octet cu octet la pozitia h[i]
                fread(&(*h)[i],1,1,fin);

    fseek(fin,54,SEEK_SET); ///ne pozitionam dupa header ( e 54 pt ca numerotarea incepe de la 0 )

    for ( int i = 0; i < height * width; i++){ ///citim restul imaginii octet cu octet la pozitia v[i]
                fread(&(*v)[i].b,1,1,fin);
                fread(&(*v)[i].g,1,1,fin);
                fread(&(*v)[i].r,1,1,fin);

    }
    *hei=height;
    *wid=width;
    fclose(fin);
}
RGB **Forma_matriceala( char *nume_fisier,HEADER **h, int *hei, int *wid)
{

    int height, width;

    *h = (HEADER*)malloc(54 * sizeof(HEADER));

    FILE *fin;

    fin=fopen(nume_fisier,"rb");

    if(fin==NULL)
        printf("Incarcare esuata\n");
    else
        printf("Incarcare reusita\n");


    fseek(fin, 18, SEEK_SET);
                                 ///se citesc dimansiuniile imaginii
    fread(&width, sizeof(unsigned int), 1, fin);
    fread(&height, sizeof(unsigned int), 1, fin);

    int padding=width%4;
                                    ///se aloca matricea dinamic si se adauga padding ul
    RGB **v;
    v = (RGB**)malloc(height * sizeof(RGB*));
    for(int i=0;i<height;i++)
        v[i]=malloc((width+padding)*sizeof(RGB));

    fseek(fin,0,SEEK_SET);

    for ( int i = 0; i < 54; i++) ///citim headerul octet cu octet la pozitia h[i]
                fread(&(*h)[i],1,1,fin);

    fseek(fin,54,SEEK_SET); ///ne pozitionam dupa header ( e 54 pt ca numerotarea incepe de la 0 )

    for ( int i = 0; i < height ; i++)
        for(int j=0;j < width+padding; j++){ ///citim restul imaginii octet cu octet la pozitia v[i]
            fread(&(v)[i][j].b,1,1,fin);
            fread(&(v)[i][j].g,1,1,fin);
            fread(&(v)[i][j].r,1,1,fin);

    }
    *hei=height;
    *wid=width;
    fclose(fin);

    return v;
}

void salvare_ext(char *nume_fisier_ext, RGB *v, HEADER *h, int hei, int wid)
{
    int height=hei, width=wid;

    FILE *fout=fopen(nume_fisier_ext,"wb");

    for ( int i = 0; i < 54; i++) ///rescriem headerul octet cu octet
            fwrite(&h[i],1,1,fout);

    for ( int i = 0; i < height * width; i++){ ///rescriem restul imaginii
                fwrite(&v[i].b,1,1,fout);
                fwrite(&v[i].g,1,1,fout);
                fwrite(&v[i].r,1,1,fout);
    }

    fflush(fout);
    fclose(fout);
}

void FisherYates(unsigned int *player, int n, unsigned int *R)
{
     int i, j, tmp;
     int k=0;
     for (i = n - 1; i > 0; i--) { /// for pentru amestecare
         j = R[++k] % (i + 1); /// j aletor  pentru algoritmul Fisher Yates
         tmp = player[j];
         player[j]= player[i];
         player[i]= tmp;
     }
}
int byte_for_int(int numar, int byte)   ///returneaza octetul cu numarul "byte" din reprezentarea unui numar intreg pe 32 de biti
{                                       ///pornind de la cel mai nesemnificativ octet
    int x = (numar >> (8*byte)) & 0xff;

    return x;
}
void criptare(char *nume_fisier_sursa, char *nume_fisier_criptat, char *key, int hei, int wid)
{
    FILE *fin=fopen(key,"r");
    if(fin==NULL)
        printf("incarcare esuata\n");
    else
        printf("incarcare reusita\n");

    unsigned int R0, SV, *R;
    fscanf(fin,"%d%d",&R0, &SV);
    fclose(fin);

    R=(unsigned int*)malloc(2*hei*wid*sizeof(unsigned int)); ///Se aloca memorie pentru generarea numerelor aleatoare
    R[0]=R0;
    for(int i=1;i<=2*hei*wid-1;i++){
        R[i]=XORSHIFT32(R0);
        R0=R[i];
    }
    unsigned int *permutare=(unsigned int*)malloc(hei*wid*sizeof(unsigned int)); ///Se aloca memorie pentru generarea permutarilor
    for(int i=0;i<hei*wid;i++)
        permutare[i]=i;
    FisherYates(permutare,hei*wid,R);

    RGB *v  = NULL;
    HEADER *h = NULL;

    Liniarizare(nume_fisier_sursa,&v,&h,&hei,&wid);///liniarizam imaginea sursa

    RGB *P = NULL;
    HEADER *H = NULL;
    H = (HEADER*)malloc(54 * sizeof(HEADER));
    P = (RGB*)malloc(hei*wid * sizeof(RGB));
    for(int i=0;i<54;i++)
        H[i]=h[i];
    for(int i=0;i<hei*wid;i++)///se copiaza imaginea permutata in P
        P[permutare[i]]=v[i];

    RGB *C;
    C = (RGB*)malloc(hei*wid * sizeof(RGB));    ///se aplica algoritmul de criptare si se salveaza rezultatul in C
    C[0].b=byte_for_int(SV,0)^P[0].b^byte_for_int(R[hei*wid],0);
    C[0].g=byte_for_int(SV,1)^P[0].g^byte_for_int(R[hei*wid],1);
    C[0].r=byte_for_int(SV,2)^P[0].r^byte_for_int(R[hei*wid],2);

    for(int i=1;i<hei*wid;i++){
    C[i].b=P[i-1].b^P[i].b^byte_for_int(R[hei*wid+i],0);
    C[i].g=P[i-1].g^P[i].g^byte_for_int(R[hei*wid+i],1);
    C[i].r=P[i-1].r^P[i].r^byte_for_int(R[hei*wid+i],2);
    }

    salvare_ext(nume_fisier_criptat,C,H,hei,wid);///se scrie imaginea criptata in nume_fisier_criptat

    free(P);    ///eliberam memoria pe care nu o mai folosim
    free(C);
    free(permutare);
    free(h);
    free(H);

}

void decriptare(char *nume_fisier_decriptat, char *nume_fisier_criptat, char *key, int hei, int wid)
{
    FILE *fkey=fopen(key,"r");
    if(fkey==NULL)
        printf("eroare la deschidere\n");
    else
        printf("incarare reusita\n");

    unsigned int R0, SV, *R;
    fscanf(fkey,"%d%d",&R0, &SV);///citim seed-ul si StartValue
    fclose(fkey);
    R=(unsigned int*)malloc(2*hei*wid*sizeof(unsigned int));
    R[0]=R0;
    for(int i=1;i<=2*hei*wid-1;i++){   ///genrarea numerelor aleatoare
        R[i]=XORSHIFT32(R0);
        R0=R[i];
    }

    unsigned int *permutare=(unsigned int*)malloc(hei*wid*sizeof(unsigned int));
    for(int i=0;i<hei*wid;i++)
        permutare[i]=i;
    FisherYates(permutare,hei*wid,R);///permutam

    unsigned int *permutare_inversa=(unsigned int*)malloc(hei*wid*sizeof(unsigned int));///generam permutarea inversa
    for(int i=0;i<hei*wid;i++)
        permutare_inversa[permutare[i]]=i;

    RGB *v  = NULL;
    HEADER *h = NULL;
    Liniarizare(nume_fisier_criptat,&v,&h,&hei,&wid);
    RGB *P = NULL;
    HEADER *H = NULL;
    H = (HEADER*)malloc(54 * sizeof(HEADER));
    P = (RGB*)malloc(hei*wid * sizeof(RGB));
    for(int i=0;i<54;i++)///copiem headarul, respectiv forma liniarizata a imaginii in H si P
        H[i]=h[i];
    for(int i=0;i<hei*wid;i++)
        P[i]=v[i];
    ///aplicarea algortmului de decriptare
    P[0].b=byte_for_int(SV,0)^P[0].b^byte_for_int(R[hei*wid],0);
    P[0].g=byte_for_int(SV,1)^P[0].g^byte_for_int(R[hei*wid],1);
    P[0].r=byte_for_int(SV,2)^P[0].r^byte_for_int(R[hei*wid],2);
    for(int i=1;i<hei*wid;i++){
        P[i].b=P[i-1].b^P[i].b^byte_for_int(R[hei*wid+i],0);
        P[i].g=P[i-1].g^P[i].g^byte_for_int(R[hei*wid+i],1);
        P[i].r=P[i-1].r^P[i].r^byte_for_int(R[hei*wid+i],2);
    }
    ///permutam invers
    RGB *C = NULL;
    C = (RGB*)malloc(hei*wid * sizeof(RGB));
    for(int i=0;i<hei*wid;i++)
        C[permutare_inversa[i]]=P[i];

    salvare_ext(nume_fisier_decriptat,C,H,hei,wid);///salvarea imaginii decriptate in "nume_fisier_decriptat"

    free(C);    ///eliberam memoria folosita
    free(P);
    free(permutare);
    free(permutare_inversa);
    free(h);
    free(H);

}

void chi_patrat(char *nume_fisier)
{
    double chi_b=0,chi_g=0,chi_r=0,f_barat;
    double *f;
    f=(double *)malloc(256*sizeof(double));
   for(int i=0;i<256;i++)///initializam fiecre f[i] declarat anterior cu 0
        f[i]=0.0;
    int hei, wid;
    RGB *v=NULL;
    HEADER *h=NULL;
    Liniarizare(nume_fisier,&v,&h,&hei,&wid);

    f_barat=hei*wid/256;
    for(int i=0;i<=255;i++)
        for(int j=0;j<hei*wid;j++)
            if(v[j].b==i)
                f[i]++;
    for(int i=0;i<256;i++)
        chi_b+=1.*((f[i]-f_barat)*(f[i]-f_barat)/f_barat);

    printf("chi pentru b este %f\n", chi_b);

    for(int i=0;i<256;i++)
        f[i]=0.0;
    for(int i=0;i<=255;i++)
        for(int j=0;j<hei*wid;j++)
            if(v[j].g==i)
                f[i]++;
    for(int i=0;i<256;i++)
        chi_g+=1.*((f[i]-f_barat)*(f[i]-f_barat)/f_barat);

    printf("chi pentru g este %f\n", chi_g);

    for(int i=0;i<256;i++)
        f[i]=0.0;
    for(int i=0;i<=255;i++)
        for(int j=0;j<hei*wid;j++)
            if(v[j].r==i)
                f[i]++;
    for(int i=0;i<256;i++)
        chi_r+=1.*((f[i]-f_barat)*(f[i]-f_barat)/f_barat);

    printf("chi pentru r este %f\n", chi_r);

    free(f);///eliberam memoria
    free(v);
    free(h);

}

void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
   FILE *fin, *fout;
   unsigned int dim_img, latime_img, inaltime_img;
   unsigned char pRGB[3], header[54], aux;

   printf("nume_fisier_sursa = %s \n",nume_fisier_sursa);

   fin = fopen(nume_fisier_sursa, "rb");
   if(fin == NULL)
   	{
   		printf("nu am gasit imaginea sursa din care citesc");
   		return;
   	}

   fout = fopen(nume_fisier_destinatie, "wb+");

   fseek(fin, 2, SEEK_SET);
   fread(&dim_img, sizeof(unsigned int), 1, fin);
   printf("Dimensiunea imaginii in octeti: %u\n", dim_img);

   fseek(fin, 18, SEEK_SET);
   fread(&latime_img, sizeof(unsigned int), 1, fin);
   fread(&inaltime_img, sizeof(unsigned int), 1, fin);
   printf("Dimensiunea imaginii in pixeli (latime x inaltime): %u x %u\n",latime_img, inaltime_img);

   //copiaza octet cu octet imaginea initiala in cea noua
	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
	fclose(fin);

	//calculam padding-ul pentru o linie
	int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;

    printf("padding = %d \n",padding);

	fseek(fout, 54, SEEK_SET);
	int i,j;
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			//citesc culorile pixelului
			fread(pRGB, 3, 1, fout);
			//fac conversia in pixel gri
			aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
			pRGB[0] = pRGB[1] = pRGB[2] = aux;
        	fseek(fout, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fout);
        	fflush(fout);
		}
		fseek(fout,padding,SEEK_CUR);
	}
	fclose(fout);
}

RGB **color(RGB **imagine,COORDONATE fereastra, RGB culoare, int hsablon, int wsablon)
{
    ///functia pentru colorarea conturului
    for(int i=0;i<wsablon;i++){
        imagine[fereastra.x][fereastra.y+i].b=culoare.b;
        imagine[fereastra.x][fereastra.y+i].g=culoare.g;
        imagine[fereastra.x][fereastra.y+i].r=culoare.r;
        imagine[fereastra.x+hsablon-1][fereastra.y+i].b=culoare.b;
        imagine[fereastra.x+hsablon-1][fereastra.y+i].g=culoare.g;
        imagine[fereastra.x+hsablon-1][fereastra.y+i].r=culoare.r;
        }
    for(int i=0;i<hsablon;i++){
        imagine[fereastra.x+i][fereastra.y].b=culoare.b;
        imagine[fereastra.x+i][fereastra.y].g=culoare.g;


        imagine[fereastra.x+i][fereastra.y].r=culoare.r;
        imagine[fereastra.x+i][fereastra.y+wsablon-1].b=culoare.b;
        imagine[fereastra.x+i][fereastra.y+wsablon-1].g=culoare.g;
        imagine[fereastra.x+i][fereastra.y+wsablon-1].r=culoare.r;
        }
    return imagine;
}

double corr(RGB **sablon, RGB **fereastra, int hsablon, int wsablon, int hifereastra, int wifereastra, int hffereastra, int wffereastra)
{
    ///functia returneaza valoare corelatiei dintre 2 imagini
    double n;
    n=hsablon*wsablon;
    double S_barat=0;
    for(int i=0;i<hsablon;i++)
        for(int j=0; j<wsablon; j++)
            S_barat=S_barat+(double)sablon[i][j].r;
    S_barat/=n;

    double sigma_S=0;
    for(int i=0;i<hsablon;i++)
        for(int j=0;j<wsablon;j++)
            sigma_S=sigma_S+(1.*((double)sablon[i][j].r-S_barat)*((double)sablon[i][j].r-S_barat));
    sigma_S/=1.*(n-1);

    double tmp;
    tmp=sqrt(sigma_S);
    sigma_S=tmp;

    double fi_barat=0;
    for(int i=hifereastra; i<hffereastra; i++)
        for(int j=wifereastra; j<wffereastra; j++)
            fi_barat=fi_barat+(double)fereastra[i][j].r;
    fi_barat/=n;

    double sigma_fi=0;
    for(int i=hifereastra;i<hffereastra;i++)
        for(int j=wifereastra;j<wffereastra;j++)
            sigma_fi=sigma_fi+(1.*((double)fereastra[i][j].r-fi_barat)*((double)fereastra[i][j].r-fi_barat));
    sigma_fi/=1.*(n-1);
    tmp=sqrt(sigma_fi);
    sigma_fi=tmp;

    double correlation=0;
    for(int i=0; i<hsablon; i++)
        for(int j=0; j<wsablon; j++)
            correlation=correlation+(1.*((double)fereastra[hifereastra+i][wifereastra+j].r-fi_barat)*((double)sablon[i][j].r-S_barat)/(sigma_fi*sigma_S));
    correlation/=1.*n;

    return correlation;
}

void template_matching(RGB **imagine, RGB **sablon, double ps, int himagine, int wimagine, int hsablon, int wsablon, DETECTIE **detectii, int *nr_detectii, int cif)
{
    ///functia parcurge fiecare pozitie (i,j) pentru care sablonul "incape" in imagine
    ///pentru fiecare corelatie mai mare decat pragul ps se vor salva coordonatele si corelatia
    ///intr-un vector de structuri cu aceste propietati
    for(int i=0;i<himagine-hsablon;i++)
        for(int j=0;j<wimagine-wsablon;j++)
        {
            double corelatie=corr(sablon,imagine,hsablon,wsablon,i,j,i+hsablon,j+wsablon);
            if(corelatie> ps){
                (*nr_detectii)++;
                (*detectii)=(DETECTIE *)realloc((*detectii),(*nr_detectii)*sizeof(DETECTIE));
                if(detectii!=NULL){///nr_detectii-1 deoarece numerotarea incepe de la 0
                    (*detectii)[*nr_detectii-1].cif=cif;
                    (*detectii)[*nr_detectii-1].x=i;
                    (*detectii)[*nr_detectii-1].y=j;
                    (*detectii)[*nr_detectii-1].ps=corelatie;
                }
                else{///cazul in care alocarea memoriei nu se produce corect
                    free(detectii);
                    printf("Eroare la realocare\n");
                    exit (1);
                }
            }
        }
}

void mat_to_img(RGB ** imagine,HEADER *headimagine, int himagine, int wimagine, char *nume_img_modif)
{
    ///scrie imaginea din forma matriceala in nume_img_modif
    FILE *fout=fopen(nume_img_modif,"wb");

    for ( int i = 0; i < 54; i++) ///rescriem headerul octet cu octet
            fwrite(&headimagine[i],1,1,fout);
    fflush(fout);
    int padding=wimagine%4;
    for ( int i = 0; i < himagine ; i++)
        for(int j = 0; j < wimagine+padding; j++){ ///rescriem restul imaginii
                fwrite(&imagine[i][j].b,1,1,fout);
                fwrite(&imagine[i][j].g,1,1,fout);
                fwrite(&imagine[i][j].r,1,1,fout);
        }

    fflush(fout);

    fclose(fout);
}
int cmp_ps(const void *t1, const void *t2)
{
    ///functia de comparare in functie de corelatie
    DETECTIE *T1= (DETECTIE *)t1;
    DETECTIE *T2= (DETECTIE *)t2;

    if((*T1).ps<(*T2).ps)
        return 1;
    if((*T1).ps>(*T2).ps)
        return -1;
    return 0;
}
double suprapunere(DETECTIE d1, DETECTIE d2, int hsablon, int wsablon)
{
    ///functia calculeaza coeficientul de suprapunere a detectiilor
    if(abs(d1.x-d2.x)>hsablon || abs(d1.y-d2.y)>wsablon)
        return 0;
    double arie_intersectie=(hsablon-abs(d1.x-d2.x))*(wsablon-abs(d1.y-d2.y));
    double x=arie_intersectie/(2*hsablon*wsablon-arie_intersectie);

    return x;
}
void eliminare_nonmax(DETECTIE **detectii, int *nr_detectii, int hsablon, int wsablon)
{
    for(int i=0;i<(*nr_detectii);i++)
        for(int j=i+1;j<(*nr_detectii);j++)
            if(suprapunere((*detectii)[i],(*detectii)[j],hsablon,wsablon)>=0.2){
                for(int l=j+1;l<(*nr_detectii);l++)
                    (*detectii)[l-1]=(*detectii)[l];
                (*nr_detectii)--;
            }
    ///se elibereaza spatiul neutilizat
    (*detectii)=realloc((*detectii),((*nr_detectii))*sizeof(DETECTIE));
}
int main(){

    RGB *v  = NULL, **imagine=NULL;
    RGB **sablon0=NULL,**sablon1=NULL,**sablon2=NULL,**sablon3=NULL,**sablon4=NULL,**sablon5=NULL,**sablon6=NULL,**sablon7=NULL,**sablon8=NULL,**sablon9=NULL;
    HEADER *h = NULL, *headimagine=NULL, *headsablon=NULL;
    DETECTIE *detectii=NULL;

    int hei,wid,hsablon,wsablon,himagine,wimagine,nr_detectii;
    char *nume_fisier_decriptat=malloc(30*sizeof(char));
    char *nume_fisier=malloc(30*sizeof(char));
    char *nume_fisier_ext=malloc(30*sizeof(char));
    char *key=malloc(30*sizeof(char));

    printf("nume fisier= ");
    scanf("%s",nume_fisier);
    //nume_fisier=realloc(nume_fisier,strlen(nume_fisier));

    printf("\nnume fisier criptat= ");
    scanf("%s",nume_fisier_ext);
    //nume_fisier_ext=realloc(nume_fisier_ext,strlen(nume_fisier_ext));

    printf("\nnume fisier decriptat= ");
    scanf("%s",nume_fisier_decriptat);
   // nume_fisier_decriptat=realloc(nume_fisier_decriptat,strlen(nume_fisier_decriptat));

    printf("\nnume fisier key= ");
    scanf("%s",key);
    //key=realloc(key,strlen(key));
    char *imagine_init="test.bmp";
    char *imagine_gray="test_grayscale.bmp";
    char *imagine_after="test123.bmp";
    char *cifra_gray0="cifra0_grayscale.bmp";
    char *cifra_gray1="cifra1_grayscale.bmp";
    char *cifra_gray2="cifra2_grayscale.bmp";
    char *cifra_gray3="cifra3_grayscale.bmp";
    char *cifra_gray4="cifra4_grayscale.bmp";
    char *cifra_gray5="cifra5_grayscale.bmp";
    char *cifra_gray6="cifra6_grayscale.bmp";
    char *cifra_gray7="cifra7_grayscale.bmp";
    char *cifra_gray8="cifra8_grayscale.bmp";
    char *cifra_gray9="cifra9_grayscale.bmp";
    Liniarizare(nume_fisier,&v,&h,&hei,&wid);

    criptare(nume_fisier,nume_fisier_ext,key,hei,wid);

    decriptare(nume_fisier_decriptat,nume_fisier_ext,key,hei,wid);

    printf("Valorile testului chi pentru imaginea initiala sunt:\n");
    chi_patrat(nume_fisier);
    printf("Valorile testului chi pentru imaginea criptata sunt:\n");
    chi_patrat(nume_fisier_ext);

    imagine=Forma_matriceala(imagine_gray,&headimagine,&himagine,&wimagine);

    nr_detectii=0;
    double ps=0.5;
    ///implementarea template matching ului pentru fiecare sablon in parte
    sablon0=Forma_matriceala(cifra_gray0,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon0,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,0);

    sablon1=Forma_matriceala(cifra_gray1,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon1,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,1);

    sablon2=Forma_matriceala(cifra_gray2,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon2,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,2);

    sablon3=Forma_matriceala(cifra_gray3,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon3,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,3);

    sablon4=Forma_matriceala(cifra_gray4,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon4,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,4);

    sablon5=Forma_matriceala(cifra_gray5,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon5,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,5);

    sablon6=Forma_matriceala(cifra_gray6,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon6,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,6);

    sablon7=Forma_matriceala(cifra_gray7,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon7,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,7);

    sablon8=Forma_matriceala(cifra_gray8,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon8,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,8);

    sablon9=Forma_matriceala(cifra_gray9,&headsablon,&hsablon,&wsablon);
    template_matching(imagine,sablon9,ps,himagine,wimagine,hsablon,wsablon,&detectii,&nr_detectii,9);


    qsort(detectii, nr_detectii, sizeof(DETECTIE), cmp_ps);
    eliminare_nonmax(&detectii,&nr_detectii,hsablon,wsablon);

    RGB *cul;///formarea vectorului de culori
    cul=malloc(10*sizeof(RGB));
    cul[0].r=255;cul[0].g=0;cul[0].b=0;
    cul[1].r=255;cul[1].g=255;cul[1].b=0;
    cul[2].r=0;cul[2].g=255;cul[2].b=0;
    cul[3].r=0;cul[3].g=255;cul[3].b=255;
    cul[4].r=255;cul[4].g=0;cul[4].b=255;
    cul[5].r=0;cul[5].g=0;cul[5].b=255;
    cul[6].r=192;cul[6].g=192;cul[6].b=192;
    cul[7].r=255;cul[7].g=140;cul[7].b=0;
    cul[8].r=128;cul[8].g=0;cul[8].b=128;
    cul[9].r=128;cul[9].g=0;cul[9].b=0;

    imagine=Forma_matriceala(imagine_init,&headimagine,&himagine,&wimagine);///se incarca imaginea color
    for(int i=0;i<nr_detectii;i++){
        COORDONATE  temp;
        temp.x=detectii[i].x;
        temp.y=detectii[i].y;
        imagine=color(imagine,temp,cul[detectii[i].cif],hsablon,wsablon);///se coloreaza detectiile ramase
    }
    mat_to_img(imagine,headimagine,himagine,wimagine,imagine_after);///se salveaza noua imagine

    return 0;
}
