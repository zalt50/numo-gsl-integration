#include "integration_internal.h"

#include <float.h>
#include <math.h>

static VALUE integration_cQAWFTable;

static double integration_qawf_cycle_length(double omega) {
  double absolute_omega = fabs(omega);

  return (2.0 * floor(absolute_omega) + 1.0) * M_PI / absolute_omega;
}

static int integration_qawf_table_double_equal(double left, double right) {
  double scale = fmax(fabs(left), fabs(right));

  return fabs(left - right) <= DBL_EPSILON * fmax(1.0, scale) * 4.0;
}

static VALUE integration_qawf_table_initialize(int argc, VALUE *argv,
                                               VALUE self) {
  VALUE options;
  VALUE omega_value;
  VALUE sine_value;
  VALUE levels_value;
  VALUE qawo_options;
  VALUE super_argv[1];
  ID keywords[] = {rb_intern("omega"), rb_intern("sine"), rb_intern("levels")};
  VALUE keyword_values[3];
  double omega;
  int sine;

  rb_scan_args(argc, argv, "0:", &options);
  if (NIL_P(options)) {
    rb_raise(rb_eArgError, "missing keywords: :omega, :sine, :levels");
  }
  rb_get_kwargs(options, keywords, 3, 0, keyword_values);
  omega_value = keyword_values[0];
  sine_value = keyword_values[1];
  levels_value = keyword_values[2];

  if (!rb_obj_is_kind_of(omega_value, rb_cNumeric)) {
    rb_raise(rb_eTypeError, "omega must be Numeric");
  }
  if (!RB_INTEGER_TYPE_P(sine_value)) {
    rb_raise(rb_eTypeError, "sine must be an Integer");
  }
  if (!RB_INTEGER_TYPE_P(levels_value)) {
    rb_raise(rb_eTypeError, "levels must be an Integer");
  }

  omega = NUM2DBL(omega_value);
  sine = NUM2INT(sine_value);
  if (!isfinite(omega) || omega == 0.0) {
    rb_raise(rb_eArgError, "omega must be finite and nonzero");
  }
  if (sine != GSL_INTEG_SINE && sine != GSL_INTEG_COSINE) {
    rb_raise(rb_eArgError, "sine must be SINE or COSINE");
  }
  if (RTEST(rb_funcall(levels_value, rb_intern("<="), 1, INT2FIX(0)))) {
    rb_raise(rb_eArgError, "levels must be positive");
  }

  qawo_options = rb_hash_new();
  rb_hash_aset(qawo_options, ID2SYM(rb_intern("omega")), omega_value);
  rb_hash_aset(qawo_options, ID2SYM(rb_intern("length")),
               DBL2NUM(integration_qawf_cycle_length(omega)));
  rb_hash_aset(qawo_options, ID2SYM(rb_intern("sine")), sine_value);
  rb_hash_aset(qawo_options, ID2SYM(rb_intern("levels")), levels_value);
  super_argv[0] = qawo_options;

  return rb_call_super_kw(1, super_argv, RB_PASS_KEYWORDS);
}

void integration_qawf_table_use_begin(VALUE table_value, double omega,
                                      enum gsl_integration_qawo_enum sine,
                                      size_t levels,
                                      integration_qawo_table_use *use) {
  double length = omega == 0.0 ? 1.0 : integration_qawf_cycle_length(omega);

  if (table_value != Qundef) {
    double table_omega;
    enum gsl_integration_qawo_enum table_sine;

    if (!rb_obj_is_kind_of(table_value, integration_cQAWFTable)) {
      rb_raise(rb_eTypeError,
               "table must be a Numo::GSL::Integration::QAWFTable");
    }

    table_omega = NUM2DBL(rb_funcall(table_value, rb_intern("omega"), 0));
    table_sine = (enum gsl_integration_qawo_enum)NUM2INT(
        rb_funcall(table_value, rb_intern("sine"), 0));
    if (!integration_qawf_table_double_equal(table_omega, omega)) {
      rb_raise(rb_eArgError, "omega does not match QAWF table");
    }
    if (table_sine != sine) {
      rb_raise(rb_eArgError, "sine does not match QAWF table");
    }
  }

  integration_qawo_table_use_begin(table_value, omega, length, sine, levels,
                                   use);
}

void integration_qawf_table_init(VALUE integration_module) {
  VALUE qawo_table_class =
      rb_const_get(integration_module, rb_intern("QAWOTable"));

  integration_cQAWFTable =
      rb_define_class_under(integration_module, "QAWFTable", qawo_table_class);
  rb_define_method(integration_cQAWFTable, "initialize",
                   integration_qawf_table_initialize, -1);
}
