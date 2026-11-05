#include "integration_internal.h"

static VALUE numo_eGslError;

static _Noreturn void integration_gsl_error_handler(const char *reason,
                                                    const char *file, int line,
                                                    int gsl_errno) {
  rb_raise(numo_eGslError, "%s:%d: %s (GSL error %d)", file, line, reason,
           gsl_errno);
}

void Init_integration(void) {
  VALUE mNumo;
  VALUE mGSL;
  VALUE mIntegration;

  mNumo = rb_define_module("Numo");
  mGSL = rb_define_module_under(mNumo, "GSL");
  mIntegration = rb_define_module_under(mGSL, "Integration");
  numo_eGslError = rb_define_class_under(mGSL, "GslError", rb_eStandardError);
  gsl_set_error_handler(&integration_gsl_error_handler);
  integration_fixed_init();

  rb_define_const(mIntegration, "GAUSS15", INT2NUM(GSL_INTEG_GAUSS15));
  rb_define_const(mIntegration, "GAUSS21", INT2NUM(GSL_INTEG_GAUSS21));
  rb_define_const(mIntegration, "GAUSS31", INT2NUM(GSL_INTEG_GAUSS31));
  rb_define_const(mIntegration, "GAUSS41", INT2NUM(GSL_INTEG_GAUSS41));
  rb_define_const(mIntegration, "GAUSS51", INT2NUM(GSL_INTEG_GAUSS51));
  rb_define_const(mIntegration, "GAUSS61", INT2NUM(GSL_INTEG_GAUSS61));
  rb_define_const(mIntegration, "COSINE", INT2NUM(GSL_INTEG_COSINE));
  rb_define_const(mIntegration, "SINE", INT2NUM(GSL_INTEG_SINE));

  rb_define_singleton_method(mIntegration, "qag", integration_qag, -1);
  rb_define_singleton_method(mIntegration, "qng", integration_qng, -1);
  rb_define_singleton_method(mIntegration, "qags", integration_qags, -1);
  rb_define_singleton_method(mIntegration, "qagp", integration_qagp, -1);
  rb_define_singleton_method(mIntegration, "qagi", integration_qagi, -1);
  rb_define_singleton_method(mIntegration, "qagiu", integration_qagiu, -1);
  rb_define_singleton_method(mIntegration, "qagil", integration_qagil, -1);
  rb_define_singleton_method(mIntegration, "qawc", integration_qawc, -1);
  rb_define_singleton_method(mIntegration, "qaws", integration_qaws, -1);
  rb_define_singleton_method(mIntegration, "qawo", integration_qawo, -1);
  rb_define_singleton_method(mIntegration, "qawf", integration_qawf, -1);
  rb_define_singleton_method(mIntegration, "cquad", integration_cquad, -1);
  rb_define_singleton_method(mIntegration, "romberg", integration_romberg, -1);
  rb_define_singleton_method(mIntegration, "glfixed", integration_glfixed, -1);
  rb_define_singleton_method(mIntegration, "fixed", integration_fixed, -1);
}
