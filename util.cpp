#include "libs.h"

Bits bits;
FILE* fp;
double c[8];
int random_access_id;
int random_access_id_base;
vector<pair<long long int, int>> pic_pos; //(pos in bitstream, last I_PIC id)

/** init **/
void init_idct(double c[8]){
	double PI = acos(-1.0);
	for(int i=1;i<=7;i++)
		c[i] = cos((PI*i)/16);
}

void init(char *filename){
	fp = fopen(filename, "rb");
	init_idct(c);
	random_access_id_base = random_access_id = -1;
}

/** bitstream process **/
uint8_t read_uint8_t(int size){
	uint8_t num=0;
	for(int i=0; i<size; i++)
		num = (num<<1) | get_next_bit();
	return num;
}
uint16_t read_uint16_t(int size){
	uint16_t num=0;
	for(int i=0; i<size; i++)
		num = (num<<1) | get_next_bit();
	return num;
}
uint32_t read_uint32_t(int size){
	uint32_t num=0;
	for(int i=0; i<size; i++){
		num = (num<<1) | get_next_bit();
	}
	return num;
}
uint8_t get_next_bit(){
	if(bits.cnt==0){
		fread(&bits.now, 1, 1, fp);
		bits.cnt=8;
	}
	return (bits.now>>(--bits.cnt))&1;
}
void clear_bit(){
	//printf("~ %d %d\n",bits.cnt, bits.now&((1<<bits.cnt)-1));
	assert((bits.now&((1<<bits.cnt)-1)) == 0);
	bits.cnt=0;
}
uint32_t seek_bits(int size){
	Bits tmp = bits;
	long pos = ftell(fp);
	uint32_t ret = read_uint32_t(size);
	bits = tmp;
	fseek(fp, pos, SEEK_SET);
	return ret;
}

/** idct **/
double t[8], a[4], b[4];
void idct_1d(double *x, double *y){
	//ref: http://blog.sina.com.cn/s/blog_4e19c4c80100gjbf.html
	t[0] = x[0] * c[4];
	t[1] = x[2] * c[2];
	t[2] = x[2] * c[6];
	t[3] = x[4] * c[4];
	t[4] = x[6] * c[6];
	t[5] = x[6] * c[2];
	a[0] = t[0] + t[1] + t[3] + t[4];
	a[1] = t[0] + t[2] - t[3] - t[5];
	a[2] = t[0] - t[2] - t[3] + t[5];
	a[3] = t[0] - t[1] + t[3] - t[4];
	b[0] = x[1]*c[1] + x[3]*c[3] + x[5]*c[5] + x[7]*c[7];
	b[1] = x[1]*c[3] - x[3]*c[7] - x[5]*c[1] - x[7]*c[5];
	b[2] = x[1]*c[5] - x[3]*c[1] + x[5]*c[7] + x[7]*c[3];
	b[3] = x[1]*c[7] - x[3]*c[5] + x[5]*c[3] - x[7]*c[1];
	y[0<<3] = a[0] + b[0];
	y[7<<3] = a[0] - b[0];
	y[1<<3] = a[1] + b[1];
	y[6<<3] = a[1] - b[1];
	y[2<<3] = a[2] + b[2];
	y[5<<3] = a[2] - b[2];
	y[3<<3] = a[3] + b[3];
	y[4<<3] = a[3] - b[3];
}
double dmat[8][8];
void idct(int mat[]){
	double tmp[64];
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
			dmat[i][j]=mat[(i<<3)+j];
	for(int i=0;i<8;i++) idct_1d((double *)dmat[i], tmp+i); // row -> col
	for(int i=0;i<8;i++) idct_1d(tmp+(i<<3), (double *)dmat+i); //row -> col
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
			mat[(i<<3)+j]=round(dmat[i][j]/4);
}
/** get current time millisecond **/
long get_current_time(){
	timeval time;
	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}
/** get picture index position in bit stream **/
int get_pic_pos(){
	uint8_t tmp8;
	uint16_t tmp16;
	uint32_t tag=0;
	pic_pos.clear();
	long long int pos = 0;
	int last_i_pos=0;
	int cur_pos=0;
	while(fread(&tmp8, 1, 1, fp)){
		pos++;
		tag = (tag << 8) + tmp8; 
		if(tag == 0x00000100){ // start of a pic
			tmp16 = 0;
			fread(&tmp16, 1, 1, fp);
			fread(&tmp8, 1, 1, fp);
			tmp16 = (tmp16<<8) + tmp8;
			tmp16 >>= 3;
			if((tmp16 & 7) == I_PIC)
				last_i_pos = cur_pos;
			pic_pos.push_back(make_pair(pos-4, last_i_pos));
			pos+=2;
			cur_pos++;
		}
	}
	fseek(fp, 0, SEEK_SET);
	return pic_pos.size();
}

void set_pic_pos(int id){
	// parsing start from last I_PIC to maintain correct forward/backward motion vector
	random_access_id = id;
	random_access_id_base = pic_pos[id].second;
	fseek(fp, pic_pos[random_access_id_base].first, SEEK_SET);
	bits.cnt = bits.now = 0;
}
