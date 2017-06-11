#include "libs.h"

int macro_addr_incr[4096];
int macro_type_I[8], macro_type_P[128], macro_type_B[128], macro_type_D[4];
int motion_VLC[4096];
int macro_pattern[1024];
int dct_dc_size_lumi[256], dct_dc_size_chromi[512];
int dct_coef[262144];

double pa_ratio_table[16] = {0.0, 1.0, 0.6735, 0.7031, 0.7615, 0.8055, 0.8437, 0.8935, 0.9375, 0.9815, 1.0255, 1.0695, 1.1250, 1.1575, 1.2051};
double pic_rate_table[16] = {0.0, 23.976, 24.0, 25.0, 29.97, 30.0, 50.0, 59.94, 60.0};
int scan[64] = {0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42, 3, 8, 12, 17, 25, 30, 41, 43, 9, 11, 18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};

void dct_coef_init(){
	for(int lst = 0; lst<2; lst++){
		int sgn = lst==0 ? 1:-1;
		dct_coef[0b10110 | lst] = sgn * ( 1 << 8 | 1);
		dct_coef[0b101000 | lst] = sgn * ( 0 << 8 | 2);
		dct_coef[0b101010 | lst] = sgn * ( 2 << 8 | 1);
		dct_coef[0b1001010 | lst] = sgn * ( 0 << 8 | 3);
		dct_coef[0b1001110 | lst] = sgn * ( 3 << 8 | 1);
		dct_coef[0b1001100 | lst] = sgn * ( 4 << 8 | 1);
		dct_coef[0b10001100 | lst] = sgn * ( 1 << 8 | 2);
		dct_coef[0b10001110 | lst] = sgn * ( 5 << 8 | 1);
		dct_coef[0b10001010 | lst] = sgn * ( 6 << 8 | 1);
		dct_coef[0b10001000 | lst] = sgn * ( 7 << 8 | 1);
		dct_coef[0b100001100 | lst] = sgn * ( 0 << 8 | 4);
		dct_coef[0b100001000 | lst] = sgn * ( 2 << 8 | 2);
		dct_coef[0b100001110 | lst] = sgn * ( 8 << 8 | 1);
		dct_coef[0b100001010 | lst] = sgn * ( 9 << 8 | 1);
		dct_coef[0b1001001100 | lst] = sgn * ( 0 << 8 | 5);
		dct_coef[0b1001000010 | lst] = sgn * ( 0 << 8 | 6);
		dct_coef[0b1001001010 | lst] = sgn * ( 1 << 8 | 3);
		dct_coef[0b1001001000 | lst] = sgn * ( 3 << 8 | 2);
		dct_coef[0b1001001110 | lst] = sgn * ( 10 << 8 | 1);
		dct_coef[0b1001000110 | lst] = sgn * ( 11 << 8 | 1);
		dct_coef[0b1001000100 | lst] = sgn * ( 12 << 8 | 1);
		dct_coef[0b1001000000 | lst] = sgn * ( 13 << 8 | 1);
		dct_coef[0b100000010100 | lst] = sgn * ( 0 << 8 | 7);
		dct_coef[0b100000011000 | lst] = sgn * ( 1 << 8 | 4);
		dct_coef[0b100000010110 | lst] = sgn * ( 2 << 8 | 3);
		dct_coef[0b100000011110 | lst] = sgn * ( 4 << 8 | 2);
		dct_coef[0b100000010010 | lst] = sgn * ( 5 << 8 | 2);
		dct_coef[0b100000011100 | lst] = sgn * ( 14 << 8 | 1);
		dct_coef[0b100000011010 | lst] = sgn * ( 15 << 8 | 1);
		dct_coef[0b100000010000 | lst] = sgn * ( 16 << 8 | 1);
		dct_coef[0b10000000111010 | lst] = sgn * ( 0 << 8 | 8);
		dct_coef[0b10000000110000 | lst] = sgn * ( 0 << 8 | 9);
		dct_coef[0b10000000100110 | lst] = sgn * ( 0 << 8 | 10);
		dct_coef[0b10000000100000 | lst] = sgn * ( 0 << 8 | 11);
		dct_coef[0b10000000110110 | lst] = sgn * ( 1 << 8 | 5);
		dct_coef[0b10000000101000 | lst] = sgn * ( 2 << 8 | 4);
		dct_coef[0b10000000111000 | lst] = sgn * ( 3 << 8 | 3);
		dct_coef[0b10000000100100 | lst] = sgn * ( 4 << 8 | 3);
		dct_coef[0b10000000111100 | lst] = sgn * ( 6 << 8 | 2);
		dct_coef[0b10000000101010 | lst] = sgn * ( 7 << 8 | 2);
		dct_coef[0b10000000100010 | lst] = sgn * ( 8 << 8 | 2);
		dct_coef[0b10000000111110 | lst] = sgn * ( 17 << 8 | 1);
		dct_coef[0b10000000110100 | lst] = sgn * ( 18 << 8 | 1);
		dct_coef[0b10000000110010 | lst] = sgn * ( 19 << 8 | 1);
		dct_coef[0b10000000101110 | lst] = sgn * ( 20 << 8 | 1);
		dct_coef[0b10000000101100 | lst] = sgn * ( 21 << 8 | 1);
		dct_coef[0b100000000110100 | lst] = sgn * ( 0 << 8 | 12);
		dct_coef[0b100000000110010 | lst] = sgn * ( 0 << 8 | 13);
		dct_coef[0b100000000110000 | lst] = sgn * ( 0 << 8 | 14);
		dct_coef[0b100000000101110 | lst] = sgn * ( 0 << 8 | 15);
		dct_coef[0b100000000101100 | lst] = sgn * ( 1 << 8 | 6);
		dct_coef[0b100000000101010 | lst] = sgn * ( 1 << 8 | 7);
		dct_coef[0b100000000101000 | lst] = sgn * ( 2 << 8 | 5);
		dct_coef[0b100000000100110 | lst] = sgn * ( 3 << 8 | 4);
		dct_coef[0b100000000100100 | lst] = sgn * ( 5 << 8 | 3);
		dct_coef[0b100000000100010 | lst] = sgn * ( 9 << 8 | 2);
		dct_coef[0b100000000100000 | lst] = sgn * ( 10 << 8 | 2);
		dct_coef[0b100000000111110 | lst] = sgn * ( 22 << 8 | 1);
		dct_coef[0b100000000111100 | lst] = sgn * ( 23 << 8 | 1);
		dct_coef[0b100000000111010 | lst] = sgn * ( 24 << 8 | 1);
		dct_coef[0b100000000111000 | lst] = sgn * ( 25 << 8 | 1);
		dct_coef[0b100000000110110 | lst] = sgn * ( 26 << 8 | 1);
		dct_coef[0b1000000000111110 | lst] = sgn * ( 0 << 8 | 16);
		dct_coef[0b1000000000111100 | lst] = sgn * ( 0 << 8 | 17);
		dct_coef[0b1000000000111010 | lst] = sgn * ( 0 << 8 | 18);
		dct_coef[0b1000000000111000 | lst] = sgn * ( 0 << 8 | 19);
		dct_coef[0b1000000000110110 | lst] = sgn * ( 0 << 8 | 20);
		dct_coef[0b1000000000110100 | lst] = sgn * ( 0 << 8 | 21);
		dct_coef[0b1000000000110010 | lst] = sgn * ( 0 << 8 | 22);
		dct_coef[0b1000000000110000 | lst] = sgn * ( 0 << 8 | 23);
		dct_coef[0b1000000000101110 | lst] = sgn * ( 0 << 8 | 24);
		dct_coef[0b1000000000101100 | lst] = sgn * ( 0 << 8 | 25);
		dct_coef[0b1000000000101010 | lst] = sgn * ( 0 << 8 | 26);
		dct_coef[0b1000000000101000 | lst] = sgn * ( 0 << 8 | 27);
		dct_coef[0b1000000000100110 | lst] = sgn * ( 0 << 8 | 28);
		dct_coef[0b1000000000100100 | lst] = sgn * ( 0 << 8 | 29);
		dct_coef[0b1000000000100010 | lst] = sgn * ( 0 << 8 | 30);
		dct_coef[0b1000000000100000 | lst] = sgn * ( 0 << 8 | 31);
		dct_coef[0b10000000000110000 | lst] = sgn * ( 0 << 8 | 32);
		dct_coef[0b10000000000101110 | lst] = sgn * ( 0 << 8 | 33);
		dct_coef[0b10000000000101100 | lst] = sgn * ( 0 << 8 | 34);
		dct_coef[0b10000000000101010 | lst] = sgn * ( 0 << 8 | 35);
		dct_coef[0b10000000000101000 | lst] = sgn * ( 0 << 8 | 36);
		dct_coef[0b10000000000100110 | lst] = sgn * ( 0 << 8 | 37);
		dct_coef[0b10000000000100100 | lst] = sgn * ( 0 << 8 | 38);
		dct_coef[0b10000000000100010 | lst] = sgn * ( 0 << 8 | 39);
		dct_coef[0b10000000000100000 | lst] = sgn * ( 0 << 8 | 40);
		dct_coef[0b10000000000111110 | lst] = sgn * ( 1 << 8 | 8);
		dct_coef[0b10000000000111100 | lst] = sgn * ( 1 << 8 | 9);
		dct_coef[0b10000000000111010 | lst] = sgn * ( 1 << 8 | 10);
		dct_coef[0b10000000000111000 | lst] = sgn * ( 1 << 8 | 11);
		dct_coef[0b10000000000110110 | lst] = sgn * ( 1 << 8 | 12);
		dct_coef[0b10000000000110100 | lst] = sgn * ( 1 << 8 | 13);
		dct_coef[0b10000000000110010 | lst] = sgn * ( 1 << 8 | 14);
		dct_coef[0b100000000000100110 | lst] = sgn * ( 1 << 8 | 15);
		dct_coef[0b100000000000100100 | lst] = sgn * ( 1 << 8 | 16);
		dct_coef[0b100000000000100010 | lst] = sgn * ( 1 << 8 | 17);
		dct_coef[0b100000000000100000 | lst] = sgn * ( 1 << 8 | 18);
		dct_coef[0b100000000000101000 | lst] = sgn * ( 6 << 8 | 3);
		dct_coef[0b100000000000110100 | lst] = sgn * ( 11 << 8 | 2);
		dct_coef[0b100000000000110010 | lst] = sgn * ( 12 << 8 | 2);
		dct_coef[0b100000000000110000 | lst] = sgn * ( 13 << 8 | 2);
		dct_coef[0b100000000000101110 | lst] = sgn * ( 14 << 8 | 2);
		dct_coef[0b100000000000101100 | lst] = sgn * ( 15 << 8 | 2);
		dct_coef[0b100000000000101010 | lst] = sgn * ( 16 << 8 | 2);
		dct_coef[0b100000000000111110 | lst] = sgn * ( 27 << 8 | 1);
		dct_coef[0b100000000000111100 | lst] = sgn * ( 28 << 8 | 1);
		dct_coef[0b100000000000111010 | lst] = sgn * ( 29 << 8 | 1);
		dct_coef[0b100000000000111000 | lst] = sgn * ( 30 << 8 | 1);
		dct_coef[0b100000000000110110 | lst] = sgn * ( 31 << 8 | 1);

	}
}

void dct_dc_size_lumi_init(){
	dct_dc_size_lumi[0b1100] = 0;
	dct_dc_size_lumi[0b100] = 1;
	dct_dc_size_lumi[0b101] = 2;
	dct_dc_size_lumi[0b1101] = 3;
	dct_dc_size_lumi[0b1110] = 4;
	dct_dc_size_lumi[0b11110] = 5;
	dct_dc_size_lumi[0b111110] = 6;
	dct_dc_size_lumi[0b1111110] = 7;
	dct_dc_size_lumi[0b11111110] = 8;
}

void dct_dc_size_chromi_init(){
	dct_dc_size_chromi[0b100] = 0;
	dct_dc_size_chromi[0b101] = 1;
	dct_dc_size_chromi[0b110] = 2;
	dct_dc_size_chromi[0b1110] = 3;
	dct_dc_size_chromi[0b11110] = 4;
	dct_dc_size_chromi[0b111110] = 5;
	dct_dc_size_chromi[0b1111110] = 6;
	dct_dc_size_chromi[0b11111110] = 7;
	dct_dc_size_chromi[0b111111110] = 8;
}

void macro_pattern_init(){
	macro_pattern[0b1111] = 60;
	macro_pattern[0b11101] = 4;
	macro_pattern[0b11100] = 8;
	macro_pattern[0b11011] = 16;
	macro_pattern[0b11010] = 32;
	macro_pattern[0b110011] = 12;
	macro_pattern[0b110010] = 48;
	macro_pattern[0b110001] = 20;
	macro_pattern[0b110000] = 40;
	macro_pattern[0b101111] = 28;
	macro_pattern[0b101110] = 44;
	macro_pattern[0b101101] = 52;
	macro_pattern[0b101100] = 56;
	macro_pattern[0b101011] = 1;
	macro_pattern[0b101010] = 61;
	macro_pattern[0b101001] = 2;
	macro_pattern[0b101000] = 62;
	macro_pattern[0b1001111] = 24;
	macro_pattern[0b1001110] = 36;
	macro_pattern[0b1001101] = 3;
	macro_pattern[0b1001100] = 63;
	macro_pattern[0b10010111] = 5;
	macro_pattern[0b10010110] = 9;
	macro_pattern[0b10010101] = 17;
	macro_pattern[0b10010100] = 33;
	macro_pattern[0b10010011] = 6;
	macro_pattern[0b10010010] = 10;
	macro_pattern[0b10010001] = 18;
	macro_pattern[0b10010000] = 34;
	macro_pattern[0b100011111] = 7;
	macro_pattern[0b100011110] = 11;
	macro_pattern[0b100011101] = 19;
	macro_pattern[0b100011100] = 35;
	macro_pattern[0b100011011] = 13;
	macro_pattern[0b100011010] = 49;
	macro_pattern[0b100011001] = 21;
	macro_pattern[0b100011000] = 41;
	macro_pattern[0b100010111] = 14;
	macro_pattern[0b100010110] = 50;
	macro_pattern[0b100010101] = 22;
	macro_pattern[0b100010100] = 42;
	macro_pattern[0b100010011] = 15;
	macro_pattern[0b100010010] = 51;
	macro_pattern[0b100010001] = 23;
	macro_pattern[0b100010000] = 43;
	macro_pattern[0b100001111] = 25;
	macro_pattern[0b100001110] = 37;
	macro_pattern[0b100001101] = 26;
	macro_pattern[0b100001100] = 38;
	macro_pattern[0b100001011] = 29;
	macro_pattern[0b100001010] = 45;
	macro_pattern[0b100001001] = 53;
	macro_pattern[0b100001000] = 57;
	macro_pattern[0b100000111] = 30;
	macro_pattern[0b100000110] = 46;
	macro_pattern[0b100000101] = 54;
	macro_pattern[0b100000100] = 58;
	macro_pattern[0b1000000111] = 31;
	macro_pattern[0b1000000110] = 47;
	macro_pattern[0b1000000101] = 55;
	macro_pattern[0b1000000100] = 59;
	macro_pattern[0b1000000011] = 27;
	macro_pattern[0b1000000010] = 39;
}

void motion_VLC_init(){
	motion_VLC[0b100000011001] = -16;
	motion_VLC[0b100000011011] = -15;
	motion_VLC[0b100000011101] = -14;
	motion_VLC[0b100000011111] = -13;
	motion_VLC[0b100000100001] = -12;
	motion_VLC[0b100000100011] = -11;
	motion_VLC[0b10000010011] = -10;
	motion_VLC[0b10000010101] = -9;
	motion_VLC[0b10000010111] = -8;
	motion_VLC[0b100000111] = -7;
	motion_VLC[0b100001001] = -6;
	motion_VLC[0b100001011] = -5;
	motion_VLC[0b10000111] = -4;
	motion_VLC[0b100011] = -3;
	motion_VLC[0b10011] = -2;
	motion_VLC[0b1011] = -1;
	motion_VLC[0b11] = 0;
	motion_VLC[0b1010] = 1;
	motion_VLC[0b10010] = 2;
	motion_VLC[0b100010] = 3;
	motion_VLC[0b10000110] = 4;
	motion_VLC[0b100001010] = 5;
	motion_VLC[0b100001000] = 6;
	motion_VLC[0b100000110] = 7;
	motion_VLC[0b10000010110] = 8;
	motion_VLC[0b10000010100] = 9;
	motion_VLC[0b10000010010] = 10;
	motion_VLC[0b100000100010] = 11;
	motion_VLC[0b100000100000] = 12;
	motion_VLC[0b100000011110] = 13;
	motion_VLC[0b100000011100] = 14;
	motion_VLC[0b100000011010] = 15;
	motion_VLC[0b100000011000] = 16;
}

void macro_type_I_init(){
	macro_type_I[0b11] = 1;
	macro_type_I[0b101] = 0b10001;
}

void macro_type_P_init(){
	macro_type_P[0b11] = 0b01010;
	macro_type_P[0b101] = 0b00010;
	macro_type_P[0b1001] = 0b01000;
	macro_type_P[0b100011] = 0b00001;
	macro_type_P[0b100010] = 0b11010;
	macro_type_P[0b100001] = 0b10010;
	macro_type_P[0b1000001] = 0b10001;
}

void macro_type_B_init(){
	macro_type_B[0b110] = 0b01100;
	macro_type_B[0b111] = 0b01110;
	macro_type_B[0b1010] = 0b00100;
	macro_type_B[0b1011] = 0b00110;
	macro_type_B[0b10010] = 0b01000;
	macro_type_B[0b10011] = 0b01010;
	macro_type_B[0b100011] = 0b00001;
	macro_type_B[0b100010] = 0b11110;
	macro_type_B[0b1000011] = 0b11010;
	macro_type_B[0b1000010] = 0b10110;
	macro_type_B[0b1000001] = 0b10001;
}

void macro_type_D_init(){
	macro_type_D[0b11] = 1;
}

void macro_addr_incr_init(){
	macro_addr_incr[0b11] = 1;
	macro_addr_incr[0b1011] = 2;
	macro_addr_incr[0b1010] = 3;
	macro_addr_incr[0b10011] = 4;
	macro_addr_incr[0b10010] = 5;
	macro_addr_incr[0b100011] = 6;
	macro_addr_incr[0b100010] = 7;
	macro_addr_incr[0b10000111] = 8;
	macro_addr_incr[0b10000110] = 9;
	macro_addr_incr[0b100001011] = 10;
	macro_addr_incr[0b100001010] = 11;
	macro_addr_incr[0b100001001] = 12;
	macro_addr_incr[0b100001000] = 13;
	macro_addr_incr[0b100000111] = 14;
	macro_addr_incr[0b100000110] = 15;
	macro_addr_incr[0b10000010111] = 16;
	macro_addr_incr[0b10000010110] = 17;
	macro_addr_incr[0b10000010101] = 18;
	macro_addr_incr[0b10000010100] = 19;
	macro_addr_incr[0b10000010011] = 20;
	macro_addr_incr[0b10000010010] = 21;
	macro_addr_incr[0b100000100011] = 22;
	macro_addr_incr[0b100000100010] = 23;
	macro_addr_incr[0b100000100001] = 24;
	macro_addr_incr[0b100000100000] = 25;
	macro_addr_incr[0b100000011111] = 26;
	macro_addr_incr[0b100000011110] = 27;
	macro_addr_incr[0b100000011101] = 28;
	macro_addr_incr[0b100000011100] = 29;
	macro_addr_incr[0b100000011011] = 30;
	macro_addr_incr[0b100000011010] = 31;
	macro_addr_incr[0b100000011001] = 32;
	macro_addr_incr[0b100000011000] = 33;
	macro_addr_incr[0b100000001111] = -1; //stuffing
	macro_addr_incr[0b100000001000] = -2; //escape
}

int read_table(int *mp){
	int key = 1;
	while(1){
		key = (key << 1) | get_next_bit();
		int it = mp[key];
		if(it!=NaN) return it;
	}
}

int get_macro_addr_incr(){
	return read_table(macro_addr_incr);
}

int get_macro_type(int IPB){ //1: I, 2: P, 3: B
	if(IPB==1) return read_table(macro_type_I);
	else if(IPB==2) return read_table(macro_type_P);
	else if(IPB==3) return read_table(macro_type_B);
	else return read_table(macro_type_D);
}

int get_motion_code(){
	return read_table(motion_VLC);
}

int get_macro_pattern(){
	return read_table(macro_pattern);
}

int get_dct_dc_size_lumi(){
	return read_table(dct_dc_size_lumi);
}

int get_dct_dc_size_chromi(){
	return read_table(dct_dc_size_chromi);
}

void clear_arr(int *arr, int size){
	for(int i=0;i<size;i++)
		arr[i]=NaN;
}

pair<int, int> get_dct_coef_first(){
	int key = 1, it;
	while(1){
		key = (key << 1) | get_next_bit();
		it = dct_coef[key];
		if(it!=NaN) break;
		else if(key == Escape){
			int run = read_uint8_t(6);
			int level = (char)read_uint8_t(8);
			if(level == 0 || level == -128) level = (level<<1) | read_uint8_t(8);
			return make_pair(run, level);
		}
		else if(key == 0b110) { it = 1; break; }
		else if(key == 0b111) { it = -1; break; }
	}
	return make_pair(Abs(it)>>8, Sgn(it) * (Abs(it)&0xFF));
}

pair<int, int> get_dct_coef_next(){
	int key = 1, it;
	while(1){
		key = (key << 1) | get_next_bit();
		it = dct_coef[key];
		if(it!=NaN) break;
		else if(key == Escape){
			int run = read_uint8_t(6);
			int level = (char)read_uint8_t(8);
			if(level == 0 || level == -128) level = (level<<1) | read_uint8_t(8);
			return make_pair(run, level);
		}
		else if(key == 0b110) { return EOB; } //EOB
		else if(key == 0b1110) { it = 1; break; }
		else if(key == 0b1111) { it = -1; break; }
	}
	return make_pair(Abs(it)>>8, Sgn(it) * (Abs(it)&0xFF));
}

int default_intra_qm[64]={ 8, 16, 19, 22, 26, 27, 29, 34, 
	16, 16, 22, 24, 27, 29, 34, 37, 19, 22, 26, 27, 29, 34, 
	34, 38, 22, 22, 26, 27, 29, 34, 37, 40, 22, 26, 27, 29,
	32, 35, 40, 48, 26, 27, 29, 32, 35, 40, 48, 58, 26, 27, 
	29, 34, 38, 46, 56, 69, 27, 29, 35, 38, 46, 56, 69, 83 };
void set_default_intra_qm(int arr[64]){
	for(int i=0; i<64; i++)
		arr[i]=default_intra_qm[i];
}

void set_default_non_intra_qm(int arr[64]){
	for(int i=0; i<64; i++)
		arr[i]=16;
}

void table_init(){
	clear_arr(macro_addr_incr, sizeof(macro_addr_incr)/sizeof(int));
	macro_addr_incr_init();
	clear_arr(macro_type_I, sizeof(macro_type_I)/sizeof(int));
	macro_type_I_init();
	clear_arr(macro_type_P, sizeof(macro_type_P)/sizeof(int));
	macro_type_P_init();
	clear_arr(macro_type_B, sizeof(macro_type_B)/sizeof(int));
	macro_type_B_init();
	clear_arr(macro_type_D, sizeof(macro_type_D)/sizeof(int));
	macro_type_D_init();
	clear_arr(motion_VLC, sizeof(motion_VLC)/sizeof(int));
	motion_VLC_init();
	clear_arr(macro_pattern, sizeof(macro_pattern)/sizeof(int));
	macro_pattern_init();
	
	clear_arr(dct_dc_size_lumi, sizeof(dct_dc_size_lumi)/sizeof(int));
	dct_dc_size_lumi_init();
	clear_arr(dct_dc_size_chromi, sizeof(dct_dc_size_chromi)/sizeof(int));
	dct_dc_size_chromi_init();
	clear_arr(dct_coef, sizeof(dct_coef)/sizeof(int));
	dct_coef_init();
}
