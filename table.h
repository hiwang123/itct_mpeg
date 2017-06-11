using std::pair;
using std::make_pair;

#define NaN -131075
#define Sgn(a) (a<0 ? -1 : 1)
#define Abs(a) (a<0 ? -a : a)
#define EOB make_pair(-1, -1)
#define Escape 0b1000001
#define MacroEscape -2

extern double pa_ratio_table[16];
extern double pic_rate_table[16];
extern int scan[64];


void table_init(void);
int get_macro_addr_incr(void);
int get_macro_type(int IPB);
int get_motion_code(void);
int get_macro_pattern(void);
int get_dct_dc_size_lumi(void);
int get_dct_dc_size_chromi(void);
void set_default_intra_qm(int arr[64]);
void set_default_non_intra_qm(int arr[64]);
pair<int, int> get_dct_coef_first(void);
pair<int, int> get_dct_coef_next(void);
