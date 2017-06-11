using std::pair;
using std::make_pair;
using std::vector;

#define Sign(a) (a==0 ? 0 : (a<0 ? -1:1))
#define Max(a,b) (a>b ? a:b)
#define Min(a,b) (a<b ? a:b)
#define Clip(a) Max(Min(a,255),0)

#define I_PIC 1
#define P_PIC 2
#define B_PIC 3
#define D_PIC 4

struct Bits{
	int cnt;
	uint8_t now;
};

struct Pixel{
	int RGB[4];
};

struct Picture{
	uint8_t pct;
	int temp_ref = -1;
	int vbv_delay;
	uint8_t fp_forward_vector, forward_r_size, forward_f;
	uint8_t fp_backward_vector, backward_r_size, backward_f;
	Pixel **data;
	Pixel **pel, **pel_for_past, **pel_back_past; //R, G, B
};

struct Slice{
	int quant_scale;
	int dct_dc_past[4];
	int past_intra_addr;
	int recon_right_for_prev, recon_down_for_prev;
	int recon_right_back_prev, recon_down_back_prev;
	int addr_prev;
};

struct MacroBlock{
	int addr;
	int type;
	int quant, m_fwd, m_bwd, pat, intra;
	int m_h_fwd_c, m_v_fwd_c;
	int m_h_fwd_r, m_v_fwd_r;
	int m_h_bwd_c, m_v_bwd_c;
	int m_h_bwd_r, m_v_bwd_r;
	int recon_right_for, recon_down_for;
	int recon_right_back, recon_down_back;
	//blocks
	int dct_recon[6][64]; //Y, Cb, Cr
};

struct Mpeg1{
	uint16_t hsize, vsize; //horizontal_size, vertical_size
	int mb_width, mb_height;
	double pa_ratio, pic_rate;
	int intra_qm[64], non_intra_qm[64];
	uint32_t nextbits;
	Picture pic;
	Slice slice;
	MacroBlock mb;
};

void init(char *filename);
uint8_t read_uint8_t(int size);
uint16_t read_uint16_t(int size);
uint32_t read_uint32_t(int size);
uint8_t get_next_bit(void);
uint32_t seek_bits(int size);
void clear_bit(void);
void idct(int mat[]);
long get_current_time(void);
int get_pic_pos(void);
void set_pic_pos(int id);

extern int random_access_id;
extern int random_access_id_base;
