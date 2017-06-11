#include "libs.h"
using std::thread;

Mpeg1 mpeg;
Pixel zero;
void init_pic_buf(){ //init buffer for pic data
	mpeg.pic.data = (Pixel **)malloc((mpeg.vsize+1)*sizeof(Pixel *));
	for(int i=0;i<mpeg.vsize;i++)
		mpeg.pic.data[i] = (Pixel *) malloc((mpeg.hsize+1)*sizeof(Pixel));
	mpeg.pic.pel = (Pixel **)malloc((mpeg.vsize+1)*sizeof(Pixel *));
	for(int i=0;i<mpeg.vsize;i++)
		mpeg.pic.pel[i] = (Pixel *) malloc((mpeg.hsize+1)*sizeof(Pixel));
	mpeg.pic.pel_for_past = (Pixel **)malloc((mpeg.vsize+1)*sizeof(Pixel *));
	for(int i=0;i<mpeg.vsize;i++)
		mpeg.pic.pel_for_past[i] = (Pixel *) malloc((mpeg.hsize+1)*sizeof(Pixel));
	mpeg.pic.pel_back_past = (Pixel **)malloc((mpeg.vsize+1)*sizeof(Pixel *));
	for(int i=0;i<mpeg.vsize;i++)
		mpeg.pic.pel_back_past[i] = (Pixel *) malloc((mpeg.hsize+1)*sizeof(Pixel));
}
void read_sequence_header(){
	uint8_t tmp8;
	uint32_t tmp32;
	mpeg.hsize = read_uint16_t(12);
	mpeg.vsize = read_uint16_t(12);
	mpeg.mb_width = (mpeg.hsize+15)/16;
	mpeg.mb_height = (mpeg.vsize+15)/16;
	tmp8 = read_uint8_t(8);
	mpeg.pa_ratio = pa_ratio_table[tmp8>>4];
	mpeg.pic_rate = pic_rate_table[tmp8 & 0xF];
	read_uint32_t(30); //don't care
	//LIQM LNQM
	if(get_next_bit() == 1){ //LIQM, specific table
		for(int i=0; i<64; i++)
			mpeg.intra_qm[i] = read_uint8_t(8);
	}else{ //LIQM, default table
		set_default_intra_qm(mpeg.intra_qm);
	}
	if(get_next_bit() == 1){ //LNQM, specific table
		for(int i=0; i<64; i++)
			mpeg.non_intra_qm[i] = read_uint8_t(8);
	}else{ //LNQM, default table
		set_default_non_intra_qm(mpeg.non_intra_qm);
	}
	//init pic buffer
	init_pic_buf();
}

void process_dct_recon(int id, int dct_recon[], int dct_zz[]){ //rerconstruct DCT coeff.
	if(mpeg.mb.intra){ //intra-coded blocks, part2 video 2.4.4.1
		int ycc = id < 4 ? 0 : (id-3); //Y: 0, Cb: 1, Cr: 2
		int quant_scale = mpeg.slice.quant_scale;
		for(int m=0; m<64; m++){
			int i=scan[m];
			dct_recon[m] = (2 * dct_zz[i] * quant_scale * mpeg.intra_qm[m]) >> 4 ;
			if((dct_recon[m]&1) == 0)
				dct_recon[m] -= Sign(dct_recon[m]) ;
			dct_recon[m] = Min(dct_recon[m], 2047);
			dct_recon[m] = Max(dct_recon[m], -2048);
		}
		if(id > 0 && id < 4){ //non-first Y blocks
			dct_recon[0] = mpeg.slice.dct_dc_past[ycc] + (dct_zz[0] << 3);
		}else{ //first Y, Cr, Cb block
			dct_recon[0] = dct_zz[0] << 3;
			if(mpeg.mb.addr - mpeg.slice.past_intra_addr > 1)
				dct_recon[0] += 1024;
			else
				dct_recon[0] += mpeg.slice.dct_dc_past[ycc];
		}
		mpeg.slice.dct_dc_past[ycc] = dct_recon[0];
	}else{ //non-intra-coded blocks, part2 video 2.4.4.2 ~ 2.4.4.3
		int quant_scale = mpeg.slice.quant_scale;
		for(int m=0; m<64; m++){
			int i=scan[m];
			dct_recon[m] = ((2*dct_zz[i] + Sign(dct_zz[i])) * quant_scale * mpeg.non_intra_qm[m]) >> 4 ;
			if((dct_recon[m]&1) == 0)
				dct_recon[m] -= Sign(dct_recon[m]) ;
			dct_recon[m] = Min(dct_recon[m], 2047);
			dct_recon[m] = Max(dct_recon[m], -2048);
			if(dct_zz[i] == 0)
				dct_recon[m] = 0;
		}
	}
}
void read_block(int id){ // part2 video 2.4.2.8
	int dct_coeff_first, dct_coeff_next;
	int i = 0;
	int dct_zz[64];
	memset(dct_zz, 0, sizeof(dct_zz));
	if(mpeg.mb.intra){
		int dc_size;
		if(id<4){ //luminance
			dc_size = get_dct_dc_size_lumi();
		}else{ //chrominance
			dc_size = get_dct_dc_size_chromi();
		}
		if(dc_size){ //dct_dc_differential => dct_zz[0]
			int dc_diff = read_uint8_t(dc_size);
			if(dc_diff & (1<<(dc_size-1))) dct_zz[0] = dc_diff;
			else dct_zz[0] = ( -1 << (dc_size) ) | (dc_diff+1) ;
		}
	}else{
		//dct_coeff_first
		pair<int, int> dct_coef_first = get_dct_coef_first();
		int run = dct_coef_first.first;
		int level = dct_coef_first.second;
		i=run;
		dct_zz[i] = level;
	}
	if(mpeg.pic.pct != D_PIC){
		pair<int, int> dct_coef_next = get_dct_coef_next();
		while(dct_coef_next != EOB){
			int run = dct_coef_next.first;
			int level = dct_coef_next.second;
			i += run + 1;
			assert(i<64);
			dct_zz[i] = level;
			dct_coef_next = get_dct_coef_next();
		}
	}
	process_dct_recon(id, mpeg.mb.dct_recon[id], dct_zz);
	idct(mpeg.mb.dct_recon[id]);
}
void process_predict_macroblock(){ //process predictive motion vector for a macroblock
	int V = (mpeg.mb.addr / mpeg.mb_width)<<4;
	int H = (mpeg.mb.addr % mpeg.mb_width)<<4;
	int right_for, down_for, right_half_for, down_half_for;
	int right_back, down_back, right_half_back, down_half_back;

	// part2 video 2.4.4.2
    right_for = mpeg.mb.recon_right_for >> 1 ;
	down_for = mpeg.mb.recon_down_for >> 1 ;
	right_half_for = mpeg.mb.recon_right_for - 2*right_for;
	down_half_for = mpeg.mb.recon_down_for - 2*down_for;
	
	right_back = mpeg.mb.recon_right_back >> 1 ;
	down_back = mpeg.mb.recon_down_back >> 1 ;
	right_half_back = mpeg.mb.recon_right_back - 2*right_back;
	down_half_back = mpeg.mb.recon_down_back - 2*down_back;
	for(int i=V; i<V+16; i++)
		for(int j=H; j<H+16; j++){
			for(int ch = 0; ch<3; ch++){
				if(i>=mpeg.vsize || j>=mpeg.hsize) continue;
				if(mpeg.mb.intra){  // intra-macroblocks has no pel[][]
					mpeg.pic.pel[i][j].RGB[ch] = 0;
					continue;
				}
				
				//forward motion
				int sum_f = 0, cnt_f = 0;
				if(mpeg.pic.pct == P_PIC || (mpeg.pic.pct == B_PIC && mpeg.mb.m_fwd)){
					sum_f += mpeg.pic.pel_for_past[i+down_for][j+right_for].RGB[ch], cnt_f++;
					if(right_half_for)
						sum_f += mpeg.pic.pel_for_past[i+down_for][j+right_for+1].RGB[ch], cnt_f++;
					if(down_half_for)
						sum_f += mpeg.pic.pel_for_past[i+down_for+1][j+right_for].RGB[ch], cnt_f++;
					if(right_half_for && down_half_for)
						sum_f += mpeg.pic.pel_for_past[i+down_for+1][j+right_for+1].RGB[ch], cnt_f++;
				}
				if(cnt_f==0) sum_f = 0;
				else sum_f = round((double)sum_f/cnt_f);
				
				//backward motion
				int sum_b = 0, cnt_b = 0;
				if(mpeg.pic.pct == B_PIC && mpeg.mb.m_bwd){
					sum_b += mpeg.pic.pel_back_past[i+down_back][j+right_back].RGB[ch], cnt_b++;
					if(right_half_back)
						sum_b += mpeg.pic.pel_back_past[i+down_back][j+right_back+1].RGB[ch], cnt_b++;
					if(down_half_back)
						sum_b += mpeg.pic.pel_back_past[i+down_back+1][j+right_back].RGB[ch], cnt_b++;
					if(right_half_back && down_half_back)
						sum_b += mpeg.pic.pel_back_past[i+down_back+1][j+right_back+1].RGB[ch], cnt_b++;
				}
				if(cnt_b==0) sum_b = 0;
				else sum_b = round((double)sum_b/cnt_b);
				
				//average forward and backward motion
				if(cnt_b && cnt_f) mpeg.pic.pel[i][j].RGB[ch] = round(((double)sum_b+sum_f)/2);
				else if(cnt_b) mpeg.pic.pel[i][j].RGB[ch] = sum_b;
				else if(cnt_f) mpeg.pic.pel[i][j].RGB[ch] = sum_f;
				else mpeg.pic.pel[i][j].RGB[ch] = 0;
			}
		}
}
void set_predict_backward_macroblock(){ //set variables for motion vector processing
	// part2 video 2.4.4.3
	int backward_f = mpeg.pic.backward_f;
	int motion_horizontal_backward_code = mpeg.mb.m_h_bwd_c;
	int motion_vertical_backward_code = mpeg.mb.m_v_bwd_c;
	int motion_horizontal_backward_r = mpeg.mb.m_h_bwd_r;
	int motion_vertical_backward_r = mpeg.mb.m_v_bwd_r;
	int complement_horizontal_backward_r, complement_vertical_backward_r;
	int full_pel_backward_vector = mpeg.pic.fp_backward_vector;
	int right_little, right_big;
	int down_little, down_big;
	
	if (backward_f == 1 || motion_horizontal_backward_code == 0){
		complement_horizontal_backward_r = 0;
	}else{
		complement_horizontal_backward_r = backward_f - 1 - motion_horizontal_backward_r;
	}
	if (backward_f == 1 || motion_vertical_backward_code == 0) {
		complement_vertical_backward_r = 0;
	} else {
		complement_vertical_backward_r = backward_f - 1 - motion_vertical_backward_r;
	}
	right_little = motion_horizontal_backward_code * backward_f;
	if (right_little == 0) {
		right_big = 0;
	} else {
		if (right_little > 0) {
			right_little = right_little - complement_horizontal_backward_r ;
			right_big = right_little - 32 * backward_f;
		} else {
			right_little = right_little + complement_horizontal_backward_r ;
			right_big = right_little + 32 * backward_f;
		}
	}
	down_little = motion_vertical_backward_code * backward_f;
	if (down_little == 0) {
		down_big = 0;
	} else {
		if (down_little > 0) {
			down_little = down_little - complement_vertical_backward_r ;
			down_big = down_little - 32 * backward_f;
		} else {
			down_little = down_little + complement_vertical_backward_r ;
			down_big = down_little + 32 * backward_f;
		}
	}
	
	int mx = (backward_f << 4) - 1;
	int mn = -(backward_f << 4);
	int new_vector;
	new_vector = mpeg.slice.recon_right_back_prev + right_little;
	if(new_vector <= mx && new_vector >= mn)
		mpeg.mb.recon_right_back =  mpeg.slice.recon_right_back_prev + right_little;
	else
		mpeg.mb.recon_right_back =  mpeg.slice.recon_right_back_prev + right_big;
	mpeg.slice.recon_right_back_prev = mpeg.mb.recon_right_back;
	if (full_pel_backward_vector) mpeg.mb.recon_right_back <<= 1;
	
	new_vector = mpeg.slice.recon_down_back_prev + down_little;
	if(new_vector <= mx && new_vector >= mn)
		mpeg.mb.recon_down_back = mpeg.slice.recon_down_back_prev + down_little;
	else
		mpeg.mb.recon_down_back = mpeg.slice.recon_down_back_prev + down_big;
	mpeg.slice.recon_down_back_prev = mpeg.mb.recon_down_back;
	if(full_pel_backward_vector) mpeg.mb.recon_down_back <<= 1;
}
void set_predict_forward_macroblock(){ //set variables for motion vector processing
	// part2 video 2.4.4.2
	int forward_f = mpeg.pic.forward_f;
	int motion_horizontal_forward_code = mpeg.mb.m_h_fwd_c;
	int motion_vertical_forward_code = mpeg.mb.m_v_fwd_c;
	int motion_horizontal_forward_r = mpeg.mb.m_h_fwd_r;
	int motion_vertical_forward_r = mpeg.mb.m_v_fwd_r;
	int complement_horizontal_forward_r, complement_vertical_forward_r;
	int full_pel_forward_vector = mpeg.pic.fp_forward_vector;
	int right_little, right_big;
	int down_little, down_big;
	
	if (forward_f == 1 || motion_horizontal_forward_code == 0){
		complement_horizontal_forward_r = 0;
	}else{
		complement_horizontal_forward_r = forward_f - 1 - motion_horizontal_forward_r;
	}
	if (forward_f == 1 || motion_vertical_forward_code == 0) {
		complement_vertical_forward_r = 0;
	} else {
		complement_vertical_forward_r = forward_f - 1 - motion_vertical_forward_r;
	}
	right_little = motion_horizontal_forward_code * forward_f;
	if (right_little == 0) {
		right_big = 0;
	} else {
		if (right_little > 0) {
			right_little = right_little - complement_horizontal_forward_r ;
			right_big = right_little - 32 * forward_f;
		} else {
			right_little = right_little + complement_horizontal_forward_r ;
			right_big = right_little + 32 * forward_f;
		}
	}
	down_little = motion_vertical_forward_code * forward_f;
	if (down_little == 0) {
		down_big = 0;
	} else {
		if (down_little > 0) {
			down_little = down_little - complement_vertical_forward_r ;
			down_big = down_little - 32 * forward_f;
		} else {
			down_little = down_little + complement_vertical_forward_r ;
			down_big = down_little + 32 * forward_f;
		}
	}
	
	int mx = (forward_f << 4) - 1;
	int mn = -(forward_f << 4);
	int new_vector;
	new_vector = mpeg.slice.recon_right_for_prev + right_little;
	if(new_vector <= mx && new_vector >= mn)
		mpeg.mb.recon_right_for =  mpeg.slice.recon_right_for_prev + right_little;
	else
		mpeg.mb.recon_right_for =  mpeg.slice.recon_right_for_prev + right_big;
	mpeg.slice.recon_right_for_prev = mpeg.mb.recon_right_for;
	if (full_pel_forward_vector) mpeg.mb.recon_right_for <<= 1;
	
	new_vector = mpeg.slice.recon_down_for_prev + down_little;
	if(new_vector <= mx && new_vector >= mn)
		mpeg.mb.recon_down_for = mpeg.slice.recon_down_for_prev + down_little;
	else
		mpeg.mb.recon_down_for = mpeg.slice.recon_down_for_prev + down_big;
	mpeg.slice.recon_down_for_prev = mpeg.mb.recon_down_for;
	if(full_pel_forward_vector) mpeg.mb.recon_down_for <<= 1;
}
void fill_pixel(){ // transform color data and motion vector to pixels of a macroblock
	int V = (mpeg.mb.addr / mpeg.mb_width)<<4;
	int H = (mpeg.mb.addr % mpeg.mb_width)<<4;
	for(int i=0; i<16; i++)
		for(int j=0; j<16; j++){
			int v = V+i, h = H+j;
			if(v>=mpeg.vsize || h>=mpeg.hsize) continue;
			int l_id = ((i>>3)<<1)+(j>>3), l_addr = ((i&7)<<3)+(j&7);
			int c_addr = ((i>>1)<<3)+(j>>1);
			int Y = mpeg.mb.dct_recon[l_id][l_addr]; //[i%8][j%8]
			int Cb = mpeg.mb.dct_recon[4][c_addr]; //[i/2][j/2]
			int Cr = mpeg.mb.dct_recon[5][c_addr]; //[i/2][j/2]
			int pel_R = mpeg.pic.pel[v][h].RGB[0];
			int pel_G = mpeg.pic.pel[v][h].RGB[1];
			int pel_B = mpeg.pic.pel[v][h].RGB[2];
			if(mpeg.mb.intra){
				Cb-=128, Cr-=128;
			}
			double R = Y + 1.28033 * Cb + pel_R;
            double G = Y - 0.21482 * Cr - 0.38059 * Cb + pel_G;
            double B = Y + 2.12798 * Cr + pel_B;

			mpeg.pic.data[v][h].RGB[0] = Clip(round(R));
			mpeg.pic.data[v][h].RGB[1] = Clip(round(G));
			mpeg.pic.data[v][h].RGB[2] = Clip(round(B));
		}
}
void read_macroblock(){
	uint16_t tmp16;
	tmp16 = seek_bits(11);
	int esc_cnt = 0;
	int addr_incr = get_macro_addr_incr();
	//printf("		macroblock_address_increment: %d\n",addr_incr);
	while(addr_incr < 0){ // read stuffing & count escape
		if(addr_incr == MacroEscape) esc_cnt++; // an escaped macrblock
		addr_incr = get_macro_addr_incr();
	}
	//process skipped_macroblocks
	for(int i=mpeg.slice.addr_prev+1; i< mpeg.slice.addr_prev + addr_incr + esc_cnt * 33; i++){ 
		mpeg.mb.addr = i;
		//no dct coefficient
		for(int j=0; j<6; j++) 
			memset(mpeg.mb.dct_recon[j], 0, sizeof(mpeg.mb.dct_recon[j]));
		// no forward motion for P picture, reset m.v. to 0
		// no forward/backward motion for B picture, reset m.v. to previous one, equiv. to no-op 
		if(mpeg.pic.pct == P_PIC){
			mpeg.mb.intra = 0;
			mpeg.mb.recon_down_for = mpeg.mb.recon_right_for = 0;
			mpeg.slice.recon_down_for_prev = mpeg.slice.recon_right_for_prev = 0;
		}
		process_predict_macroblock();
		// save to real pixel
		fill_pixel();
	}
	// update current macroblock address
	mpeg.mb.addr = mpeg.slice.addr_prev + addr_incr + esc_cnt * 33;
	// macroblock type and related variable
	mpeg.mb.type = get_macro_type(mpeg.pic.pct);
	mpeg.mb.quant = (mpeg.mb.type>>4)&1;
	mpeg.mb.m_fwd = (mpeg.mb.type>>3)&1;
	mpeg.mb.m_bwd = (mpeg.mb.type>>2)&1;
	mpeg.mb.pat = (mpeg.mb.type>>1)&1;
	mpeg.mb.intra = (mpeg.mb.type>>0)&1;
	// part2 video 2.4.2.7
	if(mpeg.mb.quant){
		mpeg.slice.quant_scale = read_uint8_t(5);
	}
	if(mpeg.mb.m_fwd){
		mpeg.mb.m_h_fwd_c = get_motion_code();
		if(mpeg.pic.forward_f != 1 && mpeg.mb.m_h_fwd_c != 0){
			mpeg.mb.m_h_fwd_r = read_uint8_t(mpeg.pic.forward_r_size);
		}
		mpeg.mb.m_v_fwd_c = get_motion_code();
		if(mpeg.pic.forward_f != 1 && mpeg.mb.m_v_fwd_c != 0){
			mpeg.mb.m_v_fwd_r = read_uint8_t(mpeg.pic.forward_r_size);
		}
	}
	if(mpeg.mb.m_bwd){
		mpeg.mb.m_h_bwd_c = get_motion_code();
		if(mpeg.pic.backward_f != 1 && mpeg.mb.m_h_bwd_c != 0){
			mpeg.mb.m_h_bwd_r = read_uint8_t(mpeg.pic.backward_r_size);
		}
		mpeg.mb.m_v_bwd_c = get_motion_code();
		if(mpeg.pic.backward_f != 1 && mpeg.mb.m_v_bwd_c != 0){
			mpeg.mb.m_v_bwd_r = read_uint8_t(mpeg.pic.backward_r_size);
		}
	}
	//predictive macroblock, part2 video 2.4.4.2 ~ 2.4.4.3
	//get forward motion vector
	if(mpeg.mb.m_fwd){
		set_predict_forward_macroblock();
	}else{
		// no forward motion for P picture, reset m.v. to 0
		if(mpeg.pic.pct == P_PIC){
			mpeg.mb.recon_down_for = mpeg.mb.recon_right_for = 0;
			mpeg.slice.recon_down_for_prev = mpeg.slice.recon_right_for_prev = 0;
		}
		// no forward motion for B picture, reset m.v. to previous one, equiv. to no-op 
	}
	//get backward motion vector
	if(mpeg.mb.m_bwd){
		set_predict_backward_macroblock();
	}else{
		// no backward motion for B picture, reset recon to previous one, equiv. to no-op 
	}
	process_predict_macroblock();
	
	//reset dct_dc_past in non-intra and skipped macroblocks
	if(!mpeg.mb.intra || addr_incr + esc_cnt * 33 > 1)
		for(int i=0;i<3;i++) mpeg.slice.dct_dc_past[i]=1024;
	int cbp = 0;
	if(mpeg.mb.pat){
		cbp = get_macro_pattern();
	}
	//process intra or patterned blocks
	for(int i=0; i<6; i++){
		memset(mpeg.mb.dct_recon[i], 0, sizeof(mpeg.mb.dct_recon[i]));
		if((cbp & (1<<(5-i))) || mpeg.mb.intra)
			read_block(i);
	}
	if(mpeg.pic.pct == D_PIC){ //EOMB
		assert(get_next_bit() == 1);
	}
	//update intra address
	mpeg.slice.addr_prev = mpeg.mb.addr;
	if(mpeg.mb.intra){
		mpeg.slice.past_intra_addr = mpeg.mb.addr;
	}
	if(mpeg.pic.pct == B_PIC && mpeg.mb.intra){
		// B coded picure and last macroblock is intra, reset m.v. for next macroblock
		mpeg.mb.recon_down_for= mpeg.mb.recon_right_for = 0;
		mpeg.slice.recon_down_for_prev = mpeg.slice.recon_right_for_prev = 0;
		mpeg.mb.recon_down_back = mpeg.mb.recon_right_back = 0;
		mpeg.slice.recon_down_back_prev = mpeg.slice.recon_right_back_prev = 0;
	}
	// save to real pixel
	fill_pixel();
}
void read_slice(){
	uint8_t tmp8;
	int slice_vertical_position = mpeg.nextbits & 0xFF;
	mpeg.slice.quant_scale = read_uint8_t(5);
	while(get_next_bit() == 1)//EBP
		read_uint8_t(8); //EIP
	//init slice data
	mpeg.slice.past_intra_addr = -2;
	for(int i=0;i<3;i++) mpeg.slice.dct_dc_past[i]=1024;
	mpeg.slice.addr_prev = (slice_vertical_position-1)*mpeg.mb_width-1;
	mpeg.slice.recon_down_for_prev = mpeg.slice.recon_right_for_prev = 0;
	mpeg.slice.recon_down_back_prev = mpeg.slice.recon_right_back_prev = 0;
	//MacroBlock
	while(seek_bits(23)!=0){
		read_macroblock();
	}
	clear_bit();
	mpeg.nextbits = read_uint32_t(32); //next_start_code
}
void read_pic(){
	uint16_t temp_ref;
	temp_ref = read_uint16_t(10);
	mpeg.pic.pct = read_uint8_t(3);
	printf("==PICTURE %d \n     picture_coding_type: %d\n",temp_ref, mpeg.pic.pct);
	mpeg.pic.vbv_delay = read_uint16_t(16);
	// part2 video 2.4.2.5
	if(mpeg.pic.pct == P_PIC || mpeg.pic.pct == B_PIC){
		uint8_t forward = read_uint8_t(4);
		mpeg.pic.fp_forward_vector = (forward>>3)&1;
		forward &= 7;
		mpeg.pic.forward_r_size = forward-1;
		mpeg.pic.forward_f = 1 << mpeg.pic.forward_r_size;
	}
	if(mpeg.pic.pct == B_PIC){
		uint8_t backward = read_uint8_t(4);
		mpeg.pic.fp_backward_vector = (backward>>3)&1;
		backward &= 7;
		mpeg.pic.backward_r_size = backward-1;
		mpeg.pic.backward_f = 1 << mpeg.pic.backward_r_size;
	}
	while(get_next_bit() == 1)//EBP
		read_uint8_t(8); //EIP
	clear_bit();
	//3-Frame Buffers Algorithm
	//buffer display
	if(mpeg.pic.pct != B_PIC){
		// for <- back
		for(int i=0;i<mpeg.vsize;i++)
			for(int j=0;j<mpeg.hsize;j++){
				mpeg.pic.pel_for_past[i][j] = mpeg.pic.pel_back_past[i][j]; 
			}
		// display(for)
		if(mpeg.pic.temp_ref != -1){ 
			display_image(mpeg.pic.pel_for_past, mpeg.hsize, mpeg.vsize,  1000/mpeg.pic_rate);
			if(random_access_id != -1){ //current pic is resetted due to user random access
				reset_display();
				mpeg.pic.temp_ref = -1;
				mpeg.nextbits = read_uint32_t(32); 
				assert(mpeg.nextbits == 0x00000100);
				return ;
			}
		}
	}
	if(random_access_id_base < random_access_id){ 
		// parsing start from last I_PIC to random access target
		// random_access_id_base = [last I_PIC, random access target) don't display
		random_access_id_base++;
	}else{ // random_access_id_base = random access target, can display 
		random_access_id_base = random_access_id = -1;
		mpeg.pic.temp_ref = temp_ref;
	}
	//Slice(picture decoding)
	mpeg.nextbits = read_uint32_t(32);
	assert(mpeg.nextbits >= 0x00000101 && mpeg.nextbits <= 0x000001af);
	while(mpeg.nextbits >= 0x00000101 && mpeg.nextbits <= 0x000001af){
		read_slice();
	}
	//buffer display
	if(mpeg.pic.pct != B_PIC){
		// back <- cur
		for(int i=0;i<mpeg.vsize;i++)
			for(int j=0;j<mpeg.hsize;j++){
				mpeg.pic.pel_back_past[i][j] = mpeg.pic.data[i][j];
			}
	}else if(mpeg.pic.pct == B_PIC && mpeg.pic.temp_ref != -1){
		// display(cur)
		display_image(mpeg.pic.data, mpeg.hsize, mpeg.vsize, 1000/mpeg.pic_rate);
		if(random_access_id != -1){ //current pic is resetted due to user random access
			reset_display();
			mpeg.pic.temp_ref = -1;
			mpeg.nextbits = read_uint32_t(32); 
			assert(mpeg.nextbits == 0x00000100);
			return ;
		}
	}
}
void read_gop(){
	uint32_t time_code;
	time_code = read_uint32_t(25);
	uint8_t tmp8;
	get_next_bit(); //closed_gop
	get_next_bit(); //broken_link
	clear_bit();
	//Picture
	mpeg.nextbits = read_uint32_t(32);
	assert(mpeg.nextbits == 0x00000100);
	while(mpeg.nextbits == 0x00000100){
		read_pic();
	}
}
void decode(){
	//sequence
	mpeg.nextbits = read_uint32_t(32);
	assert(mpeg.nextbits == 0x000001b3);
	read_sequence_header();
	//GOP
	mpeg.nextbits = read_uint32_t(32);
	assert(mpeg.nextbits == 0x000001b8);
	while(mpeg.nextbits == 0x000001b8){
		read_gop();
	}
	assert(mpeg.nextbits == 0x000001b7);
}

int main(int argc, char** argv) 
{
	assert(argc==2);
	init(argv[1]);
	int total_pic = get_pic_pos();
	set_control_pannel(total_pic);
	init_gui();
	table_init();
	decode();
    return 0;
}
