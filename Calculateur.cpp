
#include <stdio.h>
#include <stdlib.h>
#include "Aggregator.h"
#include "Calculator.h"
#include <math.h>
#define int32 int


void Messages::getAringcMessage() {
	InMessageAgregator429 = message429_1;
}

void Messages::getAFDXMessage() {
	InMessageAgregatorAFDX = messageAFDX_1;
}

//affichage binaire s par  par 4
void affichage_binaire(int32 n)
{
	int i;
	//printf("%d en binaire : ", n);
	for (i = 31; i >= 0; i = i - 1) {
		printf("%d", (n >> i) & 1);
		if (i % 4 == 0) {
			printf(" ");
		}
	}
	printf("\n");
}

//affichage hexa
void affichage_hexa(int n)
{
	printf("0x%08.8X\n", n);
}

//calcul la valeur du bit de parit  au vue des 31 premier bits. (afin de garantir un nb impair de 1)
int32 calculParite(int32 Mot429) {		//v rifier que le Mot429 n'est pas d truit
	int32 iParite = 1;							//parit  calcul e sur les 31 premiers bits du mot
	int i;
	for (i = 0;i < 31;i++) {
		iParite ^= ((unsigned int)Mot429 & 0x00000001);
		Mot429 >>= 1;
	}
	return iParite;
}


//--------------------------BNR----------------------------------------------
//--------------------------BNRtoDecimal-------------------------------------
double BNRToDecimal(int32 mot429, int range, int sigBits) {
	int mot_dec, mot_sans29, b29, i;
	double A;
	A = 0;
	b29 = (mot429 & 0x10000000) >> 28; //acc s au bit 29
	//affichage_binaire(b29);

	mot_sans29 = (mot429 & (~0x10000000));
	//affichage_binaire(~(0x10000000));

	mot_dec = (mot_sans29 & 0x1FFFFE00) >> (29 - sigBits - 1); //decallage de 29-sigBits-1
	//affichage_binaire(mot_dec);
	if (b29 == 1) {
		A = A - range; //le bit 29   un indique qu' ajoute -range   la donn e A
	}
	else {
		A = A + 0; //on ajoute rien
	}
	//printf("A = (-range ?) %f \n",A);
	// on convertit mot_dec en decimal via produit en croix.
	for (i = 0;i < sigBits;i++) {
		A = A + (((double)range) / (pow(2, (double)sigBits - i))) * (mot_dec & 0x00000001);
		mot_dec = mot_dec >> 1;
		//printf("i = %d, A = %f \n",i,A);
	}
	//A = A + ((double)(mot_sans29*range))/(pow(2,(double)sigBits));

	return A;
}

//--------------------------DecimalToBNR-------------------------------------
int32 decimalToBNR(double A, int range, int sigBits) {
	int data429, x;
	int t;

	t = (A > 0);
	x = ((int)(pow(2, (double)sigBits) * fabs(A))) / range;
	//printf("x: %d \n", x);
	//affichage_binaire(x);
	if (t) {
		data429 = x << (29 - 1 - sigBits);
	}
	else {
		x = (~(x)) + 1; //compl ment   2 sur tout les 32 bits
		//affichage_binaire(x);
		x = x << (29 - 1 - sigBits); // d callage
		//affichage_binaire(x);
		data429 = x & (~0xE0000000);// masque pour avoir des z ros sur les bits 32,31,30.
	}

	return data429;
}

//Ajout du label 0173 en d but de trame A429
int32 BNRToLabel173(int32 motBNR) {
	int32 res;
	//on met a z ro le premier octet, puis on le met   0xDE = 0173 en octal invers 
	res = (motBNR & 0xFFFFFF00) | 0x000000DE;

	return res;
}

//Ajout du label 202 en d but de trame A429
int32 BNRToLabel202(int32 motBNR) {
	int32 res;
	//on met a z ro le premier octet, puis on le met   0x41 = 0202 en octal invers 
	res = (motBNR & 0xFFFFFF00) | 0x00000041;

	return res;
}

//Ajout du label 222 en d but de trame A429
int32 BNRToLabel222(int32 motBNR) {
	int32 res;
	//on met a z ro le premier octet, puis on le met   0x91 = 0222 en octal invers 
	res = (motBNR & 0xFFFFFF00) | 0x00000091;

	return res;
}

//--------------------------BCD----------------------------------------------
//--------------------------BCDtoDecimal-------------------------------------
//prend un mot A429 label 034 le convertit en d cimal (108.00 MHz   117.95MHz)
double label034ToDecimal(int32 a429_034) {

	//d claration des variables
	int32 data, data_dec;
	double pure;

	//cache 034 bits 29   15
	data = a429_034 & 0x1FFFC000;
	//affichage_binaire(data);

	//D calage 034 bits de 29   15   bits 15   1 
	data_dec = data >> 14;
	//affichage_binaire(data_dec);

	//conversion 034 en MHz attention ajout de 100Mhz d'office
	pure = (double)100 + ((data_dec & 0x0000F000) >> 12) * 10 + ((data_dec & 0x00000F00) >> 8) * 1 + ((data_dec & 0x000000F0) >> 4) * 0.1 + (data_dec & 0x0000000F) * 0.01;

	return pure; // Question AB : on envoie quoi   FSim : un int32 ? faire un cast ?
}


//prend un mot A429 label 035 le convertit en d cimal (108.00MHz   135.95MHz)
double label035ToDecimal(int32 a429_035) {
	//d claration des variables
	int32 data, data_dec, bit18, data_dec_true;
	double pure;

	//cache 035 bits 29   18
	data = a429_035 & 0x1FFE0000;
	//printf( "0x%08.8X \n", data035,data035);
	//affichage_binaire(data);

	//D calage 035 bits 29   17
	data_dec = data >> 17;
	//affichage_binaire(data_dec);

	//conversion 035 en MHz attention :
	// - ajout de 100Mhz d'office
	// - bit 18 ie bit 1 repr sente l'ajout de 0.05Mhz ou non
	bit18 = data_dec & 0x00000001;
	//affichage_binaire(bit18);
	data_dec_true = (data_dec & 0xFFFFFFFE) >> 1;
	//affichage_binaire(data_dec_true);
	pure = 100 + ((data_dec_true & 0x00000F00) >> 8) * 10 + ((data_dec_true & 0x000000F0) >> 4) * 1 + (data_dec_true & 0x0000000F) * 0.1 + bit18 * 0.05;

	return pure;
}

//prend un mot A429 label 033 le convertit en d cimal (108.00MHz   111.95MHz)
double label033ToDecimal(int32 a429_033) {
	return label034ToDecimal(a429_033);
}

int32 decimalToLabel034(double pure_034) {
	//d claration des variables
	double dou_depart;
	int32 int_depart, bin_15, res, res_label, res_parite;

	//passage d'un fr quence de la forme 125.37   un entier int32 valant 2537
	dou_depart = (pure_034 - 100) * 100;
	//printf("%f\n",dou_depart);
	int_depart = dou_depart + 1;
	//printf("%d\n",int_depart);

	//passage de cet entier   un binaire de la forme 0000 0000 0000 0000 0XXX XXXX XXXX XXXX (15bits de donn es pour le label 034)
	bin_15 = ((int_depart / 1000) << 12) | ((int_depart % 1000) / 100) << 8 | (((int_depart % 1000) % 100) / 10) << 4 | (((int_depart % 1000) % 100)) % 10;
	//affichage_binaire(bin_15);

	//passage de ce binaire   un autre de la forme   000X XXXX XXXX XXXX XX00 0000 0000 0000 (entre les bits 29 et 15)
	res = bin_15 << 14;

	//ajout label 0034
	res_label = res | 0x00000038;

	//ajout parite
	if (calculParite(res_label)) {
		res_parite = res_label | 0x80000000;
	}
	else {
		res_parite = res_label & 0x7FFFFFFF;
	}

	return res_parite;
}


//--------------------------DecimaltoBCD-------------------------------------
int32 decimalToLabel033(double pure_033) {
	//d claration des variables
	double dou_depart;
	int32 int_depart, bin_15, res, res_label, res_parite;

	//passage d'un fr quence de la forme 125.37   un entier int32 valant 2537
	dou_depart = (pure_033 - 100) * 100;
	//printf("%f\n",dou_depart);
	int_depart = dou_depart + 1;
	//printf("%d\n",int_depart);

	//passage de cet entier   un binaire de la forme 0000 0000 0000 0000 0XXX XXXX XXXX XXXX (15bits de donn es pour le label 034)
	bin_15 = ((int_depart / 1000) << 12) | ((int_depart % 1000) / 100) << 8 | (((int_depart % 1000) % 100) / 10) << 4 | (((int_depart % 1000) % 100)) % 10;
	//affichage_binaire(bin_15);

	//passage de ce binaire   un autre de la forme   000X XXXX XXXX XXXX XX00 0000 0000 0000 (entre les bits 29 et 15)
	res = bin_15 << 14;

	//ajout label 0033
	res_label = res | 0x000000D8;

	//ajout parite
	if (calculParite(res_label)) {
		res_parite = res_label | 0x80000000;
	}
	else {
		res_parite = res_label & 0x7FFFFFFF;
	}

	return res_parite;

}

int32 decimalToLabel035(double pure_035) {
	//d claration des variables
	double dou_depart;
	int32 int_depart, bin_11, bin_12, res, res_label, res_parite;

	//passage d'un fr quence de la forme 125.35   un entier int32 valant 2535
	dou_depart = (pure_035 - 100) * 100;
	//printf("flottant %f\n",dou_depart);
	int_depart = dou_depart + 1;
	//printf("entier %d\n",int_depart);

	//passage de cet entier   un binaire de la forme 0000 0000 0000 0000 0XXX XXXX XXXX XXXX (15bits de donn es pour le label 034)
	bin_11 = ((int_depart / 1000) << 8) | ((int_depart % 1000) / 100) << 4 | (((int_depart % 1000) % 100) / 10);

	//d callage d'un bit et ajout du bit de poids 0.05MHz
	bin_12 = (bin_11 << 1) | ((((int_depart % 1000) % 100)) % 10) / 5; //le second terme vaut 1 si la donn e de d part se termine par 0.05MHz, 0 sinon.
	//affichage_binaire(bin_12);

	//passage de ce binaire   un autre de la forme   000X XXXX XXXX XXXX XX00 0000 0000 0000 (entre les bits 29 et 15)
	res = bin_12 << 17;

	//ajout label 0035
	res_label = res | 0x000000B8;

	//ajout parite
	if (calculParite(res_label)) {
		res_parite = res_label | 0x80000000;
	}
	else {
		res_parite = res_label & 0x7FFFFFFF;
	}

	return res_parite;
}

void getArinc429Message() {

}


int main()
{	/*test :
	bits 29   15 : 010 0101 0011 0111
	bits 32   1 :  XXX0 1001 0100 1101 11XX XXXX 0011 1000 pour label 034
	en hexa par exemple : 0x094DC038
	donne la valeur pure 125.37MHz

	voir slide 28 de A429
	*/

	//test bcd->decimal-----------------
	int32 mot429 = 0x094DC038; //valeur 125.37 attendue voir slide 28 de A429
	printf("la valeur pure du mot a429 :\n");
	affichage_binaire(mot429);
	affichage_hexa(mot429);
	printf("est %f mhz\n", label035ToDecimal(mot429));


	/*
	//test decimal->bcd --------------
	double pure = 125.35;
	int32 res;
	printf("la valeur pure %f est codee en 429 par :\n",pure);

	res = decimalToLabel034(label034ToDecimal(mot429));
	affichage_hexa(res);
	affichage_binaire(res);
	*/

	//test Dec->BNR----------------------
	/*essai1
	124 degr s slide 25.
	Range +-180,SigBits 7
	valeur entr e 0X0B000000
	valeur sortie attendue 123.75

	essai2
	-25.75 C slide 23.
	Range +-512,SigBits 11
	valeur entr e 0X1F32000
	valeur sortie attendue -25.75
	*/
	/*double res;
	int32 data429;
	data429 = 0x1F320000;

	res = BNRToDecimal(data429,512,11);
	printf("valeur en binaire/hexa 429 \n");
	affichage_binaire(data429);
	affichage_hexa(data429);
	printf("Valeur reelle : %f", res);
	*//*
	double A;
	int32 res;

	A = -124;
	res = decimalToBNR(A,180,7);
	printf("Valeur reelle : %f \n", A);
	printf("valeur en binaire/hexa 429 \n");
	affichage_binaire(res);
	affichage_hexa(BNRToLabel173(res));
	affichage_hexa(BNRToLabel202(res));
	affichage_hexa(BNRToLabel222(res));


	return 0;
	*/
}


