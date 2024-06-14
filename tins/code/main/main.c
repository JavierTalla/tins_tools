#include <ATcrt/ATfileoutput.h> //Includes AT_filenames.h
#include <ATcrt/definesBufferto8.h>
#include <ATcrt/ATpaths.h>
#include <ATcrt/ATcmdline.h>
#include <ATcrt/ATfiles.h>
#include <stdio.h>
#include <stdbool.h>
#include "../include/tins.h"
#ifndef ifnzunlike
#define ifnzunlike(x) ifunlike((x)!=0)
#endif
#include "main_utils.c"
#ifndef ifnz
#define ifnz(a) if((a)!=0)
#endif

#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT16 && defined(_WIN32)
	#define main wmain
#endif

#define TIN2TIN 1
#define TIN2STL 2
#define STL2TIN 3

int main(int _unused(argc), charmain_t **_argv){
	bint bname;
	u8int OPER;
	int nret;
	struct OutDir od;
	TIN tin;
	TINPlano tinpl;
	charfile_t **argv=(charfile_t **)_argv;

#ifdef _DEBUG
	#define LETRA "d"
	getchar();
#else
	#define LETRA
#endif

	putc('\n',stderr);
#if ARGV0_IS_PROGNAME
	bname=1;
	{charfile_t *pg=get_progname(*argv);
	if(strcmpargv(pg,"tins"LETRA)==0) argv++, bname=0;}
#else
	bname=0;
#endif
	if(*argv!=NULL){
		charfile_t *pg;
		if(bname) pg=get_progname(*argv);
		else pg=*argv;
		if(strcmpargv(pg,"tin2tin"LETRA)==0) OPER=TIN2TIN;
		else if(strcmpargv(pg,"tin2stl"LETRA)==0) OPER=TIN2STL;
		else if(strcmpargv(pg,"stl2tin"LETRA)==0) OPER=STL2TIN;
		else{
			PUTerr(u8"Operación no reconocida: "); PUTerrnl((char*)pg);
			*argv=NULL;
		}
	}
	if(*argv==NULL){
		PUTerr(u8"Uso del programa:\n"
			u8"tins <operation> <tin_file> [<output_file>]\n"
			u8"\n"
			u8"<operation>: tin2stl tin2tin o stl2tin\n"
			u8"<input_file>: Nombre del fichero tin/stl a transformar.\n"
			u8"<output_file>: Nombre del fichero tin/stl a generar.\n"
			u8"Sea <file> el nombre sin la extensión. Si no se indica <output_file> el nombre del\n"
			u8"fichero creado será <file>.tin o <file>.stl según corresponda. Para la opración\n"
			u8"tin2tin, indicar <output_file> es obligatorio.\n"
		);
		return 0;
	}
	argv++;

	if(*argv==NULL || (argv[1]==NULL && OPER==TIN2TIN)){
	switch(OPER){
	case TIN2TIN:
		PUTerr(u8"Uso del programa:\n"
			u8"tin2tin <tin_file> <output_tin_file>\n"
			u8"\n"
			u8"<tin_file>: Nombre del fichero tin a transformar.\n"
			u8"<output_tin_file>: Nombre del fichero tin a generar.\n"
			u8"\"Aplana\" un fichero tin, eliminando toda estructura.\n"
		);
		break;
	case TIN2STL:
		PUTerr(u8"Uso del programa:\n"
			u8"tin2stl <tin_file> [<output_file>]\n"
			u8"\n"
			u8"<tin_file>: Nombre del fichero tin a transformar.\n"
			u8"<output_file>: Nombre del fichero stl a generar.\n"
			u8"Sea <file> el nombre sin la extensión. Si no se indica <output_file> el nombre del\n"
			u8"fichero creado será <file>.stl.\n"
		);
		break;
	case STL2TIN:
		PUTerr(u8"Uso del programa:\n"
			u8"stl2tin <stl_file> [<output_file>]\n"
			u8"\n"
			u8"<stl_file>: Nombre del fichero tin a transformar.\n"
			u8"<output_file>: Nombre del fichero tin a generar.\n"
			u8"Sea <file> el nombre sin la extensión. Si no se indica <output_file> el nombre del\n"
			u8"fichero creado será <file>.tin.\n"
		);
		break;
	}
	return 0;
	}

	nret=procesa_filename((charfile_t econst *econst **)&argv, &od);
	if(nret==1) return 0;
	if(nret!=0) return 1;

	if(OPER==STL2TIN) nret=lee_fichero_stl(od.fout8,&tinpl);
	else nret=lee_fichero_tin(od.fout8,&tin);
	ifunlike(nret!=0){
		iflike(nret!=AT_NOMEM){PUTerr(u8"Error en la lectura del fichero: "); PUTerrnl(od.fout8);}
		else PUTerr(u8"El programa se ha quedado sin memoria\n");
		return 1;
	}

	if(*argv==NULL){
		*od.ext='.';
		if(OPER==STL2TIN){od.ext[1]='t'; od.ext[2]='i'; od.ext[3]='n';}
		else{od.ext[1]='s'; od.ext[2]='t'; od.ext[3]='l';}
		od.ext[4]='\0';
	}else{
		struct OutDir odf;
		nret=procesa_filename_out((charfile_t econst *econst **)&argv, &odf);
		if(nret!=0){
			const char8_t* s;
			if(nret==1){
				if(OPER==TIN2STL) s="Se ha indicado un nombre de archivo stl a generar vacío.\n";
				else s="Se ha indicado un nombre de archivo tin a generar vacío.\n";
				PUTerr(s);
			}else{
				if(OPER==TIN2STL) s=u8"Se ha indicado un nombre de archivo stl a generar erróneo: ";
				else s=u8"Se ha indicado un nombre de archivo tin a generar erróneo: ";
				PUTerr(s); PUTerrnl(od.fout8);
			}
			if(OPER==STL2TIN) free_TINplano(&tinpl);
			else free_TIN(&tin);
			return 1;
		}
		char8_t fin[SHRT_PATH];
		strcpy8(fin,od.fout8);
		path_remove_file8(fin);
		makepath8(od.fout8,SHRT_PATH,fin,odf.fout8);
	}

	if(*argv!=NULL) PUTerr(u8"Argumentos extra en la línea de comandos ignorados\n");

	if(OPER!=STL2TIN) tinplano___tin(&tinpl,&tin);
	if(OPER==TIN2STL) fstl___tinplano(od.fout8,&tinpl);
	else escribe_tinplano(od.fout8,&tinpl);

	free_TINplano(&tinpl);
	if(OPER!=STL2TIN) free_TIN(&tin);
	return 0;
}
