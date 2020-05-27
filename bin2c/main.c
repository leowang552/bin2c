
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>  
#include <errno.h>

static bool g_s_use_upx;
static bool g_s_gen_c_file;
static const char *g_s_exe_file;

static void print_user() {
	printf("convert bin file to .c file\n");
	printf("usage: bin2c [/u][/c] file\n");
	printf("/u use upx(-9k) leave the upx in the system path directory or current directory[win]\n");
	printf("/c generate bin_data.c file && bin_data.h file, only generate .h file is default\n");
}

static void print_args(int argc, char *argv[]) {
	for (int i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}
}

static bool parse_args(int argc, char* argv[]) {
#ifdef _DEBUG
	print_args(argc, argv);
#endif
	bool ret = false;
	if (argc < 2) {
		goto PARSE_ARGS_END;
	}

	if (argc == 2 && !strcmp(argv[1], "/?")){
		goto PARSE_ARGS_END;
	}

	if (argc >4){
		goto PARSE_ARGS_END;
	}

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], ("/u"))){
			g_s_use_upx = true;
		} else if (!strcmp(argv[i], "/c")){
			g_s_gen_c_file = true;
		}else if (!strcmp(argv[i], "/uc")){
			g_s_use_upx = true;
			g_s_gen_c_file = true;
		} else {
			g_s_exe_file = argv[i];
		}
	}
	
	ret = true;
PARSE_ARGS_END:
	if (!ret){
		print_user();
	}
#ifdef _DEBUG
	system("pause");
#endif
	return ret;
}

static void run_upx(const char *file) {
	char args_buf[280] = "upx -9k ";
	strcat(args_buf, file);

	system(args_buf);
}

static long get_file_size(FILE *f) {
	long old_pos = ftell(f);
	if (fseek(f, 0, SEEK_END) != 0){
		return 0;
	}

	long size = ftell(f);
	if (size == -1){
		fseek(f, old_pos, SEEK_SET);
		return 0;
	}

	fseek(f, old_pos, SEEK_SET);
	return size;
}

static int bytes_2_str(const char *bytes, size_t in_len, char *out, size_t out_len, bool end) {
	memset(out, 0, out_len);
	if (out_len < in_len * 5){
		return 0;
	}

	size_t j = 0;
	static char hex_buf[16] = "0123456789ABCDEF";
	for (size_t i = 0; i < in_len; i++) {
		unsigned char ch = bytes[i];
		out[j++] = '0';
		out[j++] = 'X';
		out[j++] = hex_buf[ch >> 4];
		out[j++] = hex_buf[ch & 0xF];
		if (!(end && i == in_len - 1)){
			out[j++] = ',';
		}
	}

	if (end){
		out[j++] = '}';
		out[j++] = ';';
	}
	return j;
}

static void bin_2_c() {
	if (g_s_use_upx){
		run_upx(g_s_exe_file);
	}
	
	FILE *f_bin = 0;
	FILE *f_header = 0;
	FILE *f_c_file = 0;
	do {
		f_bin = fopen(g_s_exe_file, "rb");
		if (!f_bin) {
			printf("open file failed, path:%s, err:%d", g_s_exe_file, errno);
			break;
		}

		long file_size = get_file_size(f_bin);
		if (!file_size) {
			printf("seek file failed, path:%s, err:%d", g_s_exe_file, errno);
			break;
		}

		f_header = fopen("bin_data.h", "wb");
		if (!f_header) {
			printf("open file failed, path:bin_data.h, err:%d", errno);
			break;
		}

		if (g_s_gen_c_file){
			f_c_file = fopen("bin_data.c", "wb");
			if (!f_c_file) {
				printf("open file failed, path:bin_data.h, err:%d", errno);
				break;
			}
		}

		if (g_s_gen_c_file) {
			fputs("#include \"bin_data.h\"\n", f_c_file);
		}

		FILE *f_data = g_s_gen_c_file ? f_c_file : f_header;
		fprintf(f_data, "const int k_bin_file_size = %ld;\n", file_size);
		fputs("const unsigned char g_bin_data[] = {", f_data);

		char read_buf[1024] = { 0 };
		char write_buf[sizeof(read_buf) * 5 + 2] = { 0 };
		while (1) {
			bool end_file = false;
			size_t  read_size = fread(read_buf, 1, sizeof(read_buf), f_bin);
			if (feof(f_bin) || ferror(f_bin)) {
				end_file = true;
			}
			int write_size = bytes_2_str(read_buf, read_size, write_buf, sizeof(write_buf), end_file);
			int wrote_size = fwrite(write_buf, 1, write_size, f_data);
			if (wrote_size != write_size) {
				printf("write %d\n", errno);
				break;
			}

			if (end_file) {
				break;
			}
		}

		if (g_s_gen_c_file){
			fputs("extern const int k_bin_file_size;\n", f_header);
			fputs("extern const unsigned char g_bin_data[];", f_header);
		}
	} while (0);


	if (f_header){
		fflush(f_header);
		fclose(f_header);
	}

	if (f_bin){
		fclose(f_bin);
	}
	
	if (f_c_file){
		fflush(f_c_file);
		fclose(f_c_file);
	}
}


int main(int argc, char* argv[]) {
	if (!parse_args(argc, argv)){
		return 0;
	}

	bin_2_c();

	return 0;
}
