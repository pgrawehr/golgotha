#pragma once

#define FALSE 0
#define TRUE 1

#define ERROR 0
#define G1_SECTION_MODEL_QUADS 18
#define G1_SECTION_MODEL_TEXTURE_NAMES 19
#define G1_SECTION_MODEL_VERT_ANIMATION 20
#define GMOD_MAX_SECTIONS 32
#define GMOD_UNUSED_VERTEX 65535
#define PI 3.141592653589793238462643
#define SUCCESS 1

#define DEG_TO_RAD   ( PI / 180.0 )
#define RAD_TO_DEG   ( 180.0 / PI )


#undef COR3_MAX
#define COLOR_MAX 1000
#define COR3_MAX 60000
#define FACE_MAX 60000
#define LINE_MAX_LEN 256
#define LEVEL_MAX 10
#define LINE_MAX 5000
#define MATERIAL_MAX 1000
#define ORDER_MAX 10
#define TEXTURE_MAX 60000

#define FACE_FLAGS_UNKNOWN 0xff //don't use these (do not know what they are for)
#define FACE_FLAGS_BOTHSIDED 0x100
#define FACE_FLAGS_MATERIAL_ASSIGNED 0x200

typedef struct color_
{
public:
	color_()
	{
		R = 0; G = 0; B = 0; A = 0;
	}

	color_(float r, float g, float b, float a)
	{
		R = r; G = g; B = b; A = a;
	}

	float R;
	float G;
	float B;
	float A;
} color;

// Global variables (YUK!!!)
extern char anim_name[LINE_MAX_LEN];


extern float cor3[3][COR3_MAX];
extern int cor3_material[COR3_MAX];
extern float cor3_normal[3][COR3_MAX];
extern int cor3_num;

extern char material_name[MATERIAL_MAX][LINE_MAX_LEN];
extern int material_num;
extern float vertex_uv_temp[2][ORDER_MAX * FACE_MAX];
extern color material_rgba[MATERIAL_MAX];

extern int vertex_material[ORDER_MAX][FACE_MAX];
extern float vertex_normal[3][ORDER_MAX][FACE_MAX];
extern float vertex_rgb[3][ORDER_MAX][FACE_MAX];
extern float vertex_tex_uv[2][ORDER_MAX][FACE_MAX];
extern int debug;

extern float face_normal[3][FACE_MAX];
extern int face[ORDER_MAX][FACE_MAX];
extern float face_area[FACE_MAX];
extern int face_flags[FACE_MAX];
extern int face_material[FACE_MAX];
extern int face_order[FACE_MAX];
extern bool face_ignoresides;
extern int face_num;
extern int texture_num;

extern char texture_name[TEXTURE_MAX][LINE_MAX_LEN];

extern int line_dex[LINE_MAX];
extern int line_material[LINE_MAX];
extern int line_num;

extern int text_num;
extern int comment_num;
extern int bad_num;

extern char fileout_name[LINE_MAX_LEN];
extern char filein_name[LINE_MAX_LEN];
extern char object_name[81];

extern float normal_temp[3][ORDER_MAX * FACE_MAX];

/******************************************************************************/

/* FUNCTION PROTOTYPES */

/******************************************************************************/

int                main(int argc, char** argv);
int                ase_read(FILE* filein);
int                ase_write(FILE* fileout);
int                byu_read(FILE* filein);
int                byu_write(FILE* fileout);
int                char_index_last(char* string, char c);
int                char_pad(int* char_index, int* null_index, char* string,
	int STRING_MAX);
char               char_read(FILE* filein);
int                char_write(FILE* fileout, char c);
int command_line(char** argv, int argc);
void               cor3_normal_set(void);
void               cor3_range(void);
void               data_check(void);
void               data_init(void);
int                data_read(void);
void               data_report(void);
int                data_write(void);
int                dxf_read(FILE* filein);
int                dxf_write(FILE* fileout);
void               edge_null_delete(void);
void               face_area_set(bool domodifications);
void               face_normal_ave(void);
void               face_null_delete(void);
int                face_print(int iface);
void               face_reverse_order(void);
int                face_subset(void);
void               face_to_line(void);
void               face_to_vertex_material(void);
char* file_ext(char* file_name);
float              float_read(FILE* filein);
float              float_reverse_bytes(float x);
int                float_write(FILE* fileout, float float_val);
void               hello(void);
void               help(void);
int                hrc_read(FILE* filein);
int                hrc_write(FILE* fileout);
void               init_program_data(void);
int                interact(void);
int                iv_read(FILE* filein);
int                iv_write(FILE* fileout);
int                ivec_max(int n, int* a);
int                leqi(char* string1, char* string2);
long int           long_int_read(FILE* filein);
int                long_int_write(FILE* fileout, long int int_val);
void               news(void);
void               node_to_vertex_material(void);
int pov_write(FILE* fileout);
float normalize_texture_coordinates(float vt);
int                rcol_find(float a[][COR3_MAX], int m, int n, float r[]);
float              rgb_to_hue(float r, float g, float b);
short int          short_int_read(FILE* filein);
int                short_int_write(FILE* fileout, unsigned short int int_val);
int                smf_read(FILE* filein);
int                smf_write(FILE* fileout);
int                stla_read(FILE* filein);
int                stla_write(FILE* fileout);
int                stlb_read(FILE* filein);
int                stlb_write(FILE* fileout);
void               tds_pre_process(void);
int                tds_read(FILE* filein);
unsigned long int  tds_read_ambient_section(FILE* filein, unsigned long matindex);
unsigned long int  tds_read_background_section(FILE* filein);
unsigned long int  tds_read_boolean(unsigned char* boolean, FILE* filein);
unsigned long int  tds_read_camera_section(FILE* filein);
unsigned long int  tds_read_edit_section(FILE* filein, int* views_read);
unsigned long int  tds_read_keyframe_section(FILE* filein, int* views_read);
unsigned long int  tds_read_keyframe_objdes_section(FILE* filein);
unsigned long int  tds_read_light_section(FILE* filein);
unsigned long int  tds_read_u_long_int(FILE* filein);
int                tds_read_long_name(FILE* filein);
unsigned long int  tds_read_matdef_section(FILE* filein);
unsigned long int  tds_read_material_section(FILE* filein);
int                tds_read_name(FILE* filein);
unsigned long int  tds_read_obj_section(FILE* filein);
unsigned long int  tds_read_object_section(FILE* filein);
unsigned long int  tds_read_tex_verts_section(FILE* filein);
unsigned long int  tds_read_texmap_section(FILE* filein);
unsigned short int tds_read_u_short_int(FILE* filein);
unsigned long int  tds_read_spot_section(FILE* filein);
unsigned long int  tds_read_unknown_section(FILE* filein);
unsigned long int  tds_read_view_section(FILE* filein, int* views_read);
unsigned long int  tds_read_vp_section(FILE* filein, int* views_read);
int                tds_write(FILE* fileout);
int                tds_write_string(FILE* fileout, char* string);
int                tds_write_u_short_int(FILE* fileout,
	unsigned short int int_val);
int                tec_write(FILE* fileout);
void               tmat_init(float a[4][4]);
void               tmat_mxm(float a[4][4], float b[4][4], float c[4][4]);
void               tmat_mxp(float a[4][4], float x[4], float y[4]);
void               tmat_mxp2(float a[4][4], float x[][3], float y[][3], int n);
void               tmat_mxv(float a[4][4], float x[4], float y[4]);
void               tmat_rot_axis(float a[4][4], float b[4][4], float angle,
	char axis);
void               tmat_rot_vector(float a[4][4], float b[4][4], float angle,
	float v1, float v2, float v3);
void               tmat_scale(float a[4][4], float b[4][4], float sx, float sy,
	float sz);
void               tmat_shear(float a[4][4], float b[4][4], char* axis,
	float s);
void               tmat_trans(float a[4][4], float b[4][4], float x, float y,
	float z);
int                tria_read(FILE* filein);
int                tria_write(FILE* fileout);
int                trib_read(FILE* filein);
int                trib_write(FILE* fileout);
int                txt_write(FILE* fileout);
int                ucd_write(FILE* fileout);
void               vertex_normal_set(void);
void               vertex_to_face_material(void);
void               vertex_to_node_material(void);
int                vla_read(FILE* filein);
int                vla_write(FILE* fileout);
int                wrl_write(FILE* filout);
int                xgl_write(FILE* fileout);
void tga_write_materials(char* filename);
void ConvertTextureToMaterial();
void ConvertMaterialToTexture();
