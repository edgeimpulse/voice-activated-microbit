#include "AsciiKeyMap.h"
#include "USB_HID_Keys.h"

using namespace codal;

//define all Key sequences to be used
static const Key seq_backspace[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_BACKSPACE },
};

static const Key seq_tab[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_TAB },
};

static const Key seq_newline[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_ENTER },
};

static const Key seq_space[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_SPACE },
};

static const Key seq_exclamation_point[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_1 },
};

static const Key seq_quote[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_1 },
};

static const Key seq_pound[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_3 },
};

static const Key seq_dollar[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_4 },
};

static const Key seq_percent[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_5 },
};

static const Key seq_amp[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_7 },
};

static const Key seq_apostrophe[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_APOSTROPHE },
};

static const Key seq_left_paren[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_9 },
};

static const Key seq_right_paren[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_0 },
};

static const Key seq_ast[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_8 },
};

static const Key seq_plus[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_EQUAL },
};

static const Key seq_comma[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_COMMA },
};

static const Key seq_minus[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_MINUS },
};

static const Key seq_dot[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_DOT },
};

static const Key seq_slash[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_SLASH },
};

static const Key seq_0[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_0 },
};

static const Key seq_1[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_1 },
};

static const Key seq_2[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_2 },
};

static const Key seq_3[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_3 },
};

static const Key seq_4[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_4 },
};

static const Key seq_5[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_5 },
};

static const Key seq_6[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_6 },
};

static const Key seq_7[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_7 },
};

static const Key seq_8[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_8 },
};

static const Key seq_9[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_9 },
};

static const Key seq_colon[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_SEMICOLON },
};

static const Key seq_semicolon[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_SEMICOLON },
};

static const Key seq_angle_left[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_COMMA },
};

static const Key seq_equal[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_EQUAL },
};

static const Key seq_angle_right[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_DOT },
};

static const Key seq_question[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_SLASH },
};

static const Key seq_at[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_2 },
};

static const Key seq_A[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_A },
};

static const Key seq_B[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_B },
};

static const Key seq_C[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_C },
};

static const Key seq_D[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_D },
};

static const Key seq_E[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_E },
};

static const Key seq_F[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_F },
};

static const Key seq_G[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_G },
};

static const Key seq_H[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_H },
};

static const Key seq_I[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_I },
};

static const Key seq_J[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_J },
};

static const Key seq_K[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_K },
};

static const Key seq_L[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_L },
};

static const Key seq_M[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_M },
};

static const Key seq_N[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_N },
};

static const Key seq_O[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_O },
};

static const Key seq_P[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_P },
};

static const Key seq_Q[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_Q },
};

static const Key seq_R[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_R },
};

static const Key seq_S[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_S },
};

static const Key seq_T[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_T },
};

static const Key seq_U[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_U },
};

static const Key seq_V[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_V },
};

static const Key seq_W[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_W },
};

static const Key seq_X[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_X },
};

static const Key seq_Y[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_Y },
};

static const Key seq_Z[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_Z },
};

static const Key seq_brace_left[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_LEFTBRACE },
};

static const Key seq_backslash[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_BACKSLASH },
};

static const Key seq_brace_right[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_RIGHTBRACE },
};

static const Key seq_hat[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_6 },
};

static const Key seq_underscore[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_MINUS },
};

static const Key seq_grave[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_GRAVE },
};

static const Key seq_a[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_A },
};

static const Key seq_b[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_B },
};

static const Key seq_c[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_C },
};

static const Key seq_d[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_D },
};

static const Key seq_e[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_E },
};

static const Key seq_f[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_F },
};

static const Key seq_g[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_G },
};

static const Key seq_h[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_H },
};

static const Key seq_i[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_I },
};

static const Key seq_j[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_J },
};

static const Key seq_k[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_K },
};

static const Key seq_l[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_L },
};

static const Key seq_m[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_M },
};

static const Key seq_n[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_N },
};

static const Key seq_o[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_O },
};

static const Key seq_p[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_P },
};

static const Key seq_q[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_Q },
};

static const Key seq_r[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_R },
};

static const Key seq_s[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_S },
};

static const Key seq_t[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_T },
};

static const Key seq_u[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_U },
};

static const Key seq_v[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_V },
};

static const Key seq_w[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_W },
};

static const Key seq_x[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_X },
};

static const Key seq_y[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_Y },
};

static const Key seq_z[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_Z },
};

static const Key seq_curly_left[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_LEFTBRACE },
};

static const Key seq_pipe[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_BACKSLASH },
};

static const Key seq_curly_right[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_RIGHTBRACE},
};

static const Key seq_tilda[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEYMAP_MODIFIER_KEY | KEY_MOD_LSHIFT },
		{ .reg = KEYMAP_KEY_DOWN | KEY_GRAVE },
};

static const Key seq_del[] = {
		{ .reg = KEYMAP_KEY_DOWN | KEY_BACKSPACE },
};

static const KeySequence unmapped =	{
	NULL,
	0
};

//define the keymap
const KeySequence ascii_keymap[] = {
	unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, //0 - 7
	KEYMAP_REGISTER(seq_backspace),
	KEYMAP_REGISTER(seq_tab),
	KEYMAP_REGISTER(seq_newline),
	unmapped, unmapped, unmapped, unmapped, unmapped,
	unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, unmapped, //16 - 31
	KEYMAP_REGISTER(seq_space),								//32 space
	KEYMAP_REGISTER(seq_exclamation_point),					//33 !
	KEYMAP_REGISTER(seq_quote),								//34 "
	KEYMAP_REGISTER(seq_pound),								//35 #
	KEYMAP_REGISTER(seq_dollar),
	KEYMAP_REGISTER(seq_percent),
	KEYMAP_REGISTER(seq_amp),
	KEYMAP_REGISTER(seq_apostrophe),
	KEYMAP_REGISTER(seq_left_paren),
	KEYMAP_REGISTER(seq_right_paren),
	KEYMAP_REGISTER(seq_ast),
	KEYMAP_REGISTER(seq_plus),
	KEYMAP_REGISTER(seq_comma),
	KEYMAP_REGISTER(seq_minus),
	KEYMAP_REGISTER(seq_dot),
	KEYMAP_REGISTER(seq_slash),
	KEYMAP_REGISTER(seq_0),
	KEYMAP_REGISTER(seq_1),
	KEYMAP_REGISTER(seq_2),
	KEYMAP_REGISTER(seq_3),
	KEYMAP_REGISTER(seq_4),
	KEYMAP_REGISTER(seq_5),
	KEYMAP_REGISTER(seq_6),
	KEYMAP_REGISTER(seq_7),
	KEYMAP_REGISTER(seq_8),
	KEYMAP_REGISTER(seq_9),
	KEYMAP_REGISTER(seq_colon),
	KEYMAP_REGISTER(seq_semicolon),
	KEYMAP_REGISTER(seq_angle_left),
	KEYMAP_REGISTER(seq_equal),
	KEYMAP_REGISTER(seq_angle_right),
	KEYMAP_REGISTER(seq_question),
	KEYMAP_REGISTER(seq_at),
	KEYMAP_REGISTER(seq_A),
	KEYMAP_REGISTER(seq_B),
	KEYMAP_REGISTER(seq_C),
	KEYMAP_REGISTER(seq_D),
	KEYMAP_REGISTER(seq_E),
	KEYMAP_REGISTER(seq_F),
	KEYMAP_REGISTER(seq_G),
	KEYMAP_REGISTER(seq_H),
	KEYMAP_REGISTER(seq_I),
	KEYMAP_REGISTER(seq_J),
	KEYMAP_REGISTER(seq_K),
	KEYMAP_REGISTER(seq_L),
	KEYMAP_REGISTER(seq_M),
	KEYMAP_REGISTER(seq_N),
	KEYMAP_REGISTER(seq_O),
	KEYMAP_REGISTER(seq_P),
	KEYMAP_REGISTER(seq_Q),
	KEYMAP_REGISTER(seq_R),
	KEYMAP_REGISTER(seq_S),
	KEYMAP_REGISTER(seq_T),
	KEYMAP_REGISTER(seq_U),
	KEYMAP_REGISTER(seq_V),
	KEYMAP_REGISTER(seq_W),
	KEYMAP_REGISTER(seq_X),
	KEYMAP_REGISTER(seq_Y),
	KEYMAP_REGISTER(seq_Z),
	KEYMAP_REGISTER(seq_brace_left),
	KEYMAP_REGISTER(seq_backslash),
	KEYMAP_REGISTER(seq_brace_right),
	KEYMAP_REGISTER(seq_hat),
	KEYMAP_REGISTER(seq_underscore),
	KEYMAP_REGISTER(seq_grave),
	KEYMAP_REGISTER(seq_a),
	KEYMAP_REGISTER(seq_b),
	KEYMAP_REGISTER(seq_c),
	KEYMAP_REGISTER(seq_d),
	KEYMAP_REGISTER(seq_e),
	KEYMAP_REGISTER(seq_f),
	KEYMAP_REGISTER(seq_g),
	KEYMAP_REGISTER(seq_h),
	KEYMAP_REGISTER(seq_i),
	KEYMAP_REGISTER(seq_j),
	KEYMAP_REGISTER(seq_k),
	KEYMAP_REGISTER(seq_l),
	KEYMAP_REGISTER(seq_m),
	KEYMAP_REGISTER(seq_n),
	KEYMAP_REGISTER(seq_o),
	KEYMAP_REGISTER(seq_p),
	KEYMAP_REGISTER(seq_q),
	KEYMAP_REGISTER(seq_r),
	KEYMAP_REGISTER(seq_s),
	KEYMAP_REGISTER(seq_t),
	KEYMAP_REGISTER(seq_u),
	KEYMAP_REGISTER(seq_v),
	KEYMAP_REGISTER(seq_w),
	KEYMAP_REGISTER(seq_x),
	KEYMAP_REGISTER(seq_y),
	KEYMAP_REGISTER(seq_z),
	KEYMAP_REGISTER(seq_curly_left),
	KEYMAP_REGISTER(seq_pipe),
	KEYMAP_REGISTER(seq_curly_right),
	KEYMAP_REGISTER(seq_tilda),
	KEYMAP_REGISTER(seq_del),
};

AsciiKeyMap asciiKeyMap(ascii_keymap, 128);