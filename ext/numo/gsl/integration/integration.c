#include "integration_internal.h"

void Init_integration(void) {
  VALUE mNumo;
  VALUE mGSL;
  VALUE mIntegration;

  mNumo = rb_define_module("Numo");
  mGSL = rb_define_module_under(mNumo, "GSL");
  mIntegration = rb_define_module_under(mGSL, "Integration");

  rb_define_const(mIntegration, "GAUSS15", INT2NUM(GSL_INTEG_GAUSS15));
  rb_define_const(mIntegration, "GAUSS21", INT2NUM(GSL_INTEG_GAUSS21));
  rb_define_const(mIntegration, "GAUSS31", INT2NUM(GSL_INTEG_GAUSS31));
  rb_define_const(mIntegration, "GAUSS41", INT2NUM(GSL_INTEG_GAUSS41));
  rb_define_const(mIntegration, "GAUSS51", INT2NUM(GSL_INTEG_GAUSS51));
  rb_define_const(mIntegration, "GAUSS61", INT2NUM(GSL_INTEG_GAUSS61));

  rb_define_singleton_method(mIntegration, "qag", integration_qag, -1);
  rb_define_singleton_method(mIntegration, "qng", integration_qng, -1);
  rb_define_singleton_method(mIntegration, "qags", integration_qags, -1);
  rb_define_singleton_method(mIntegration, "qagp", integration_qagp, -1);
  rb_define_singleton_method(mIntegration, "qagi", integration_qagi, -1);
  rb_define_singleton_method(mIntegration, "qagiu", integration_qagiu, -1);
  rb_define_singleton_method(mIntegration, "qagil", integration_qagil, -1);
  rb_define_singleton_method(mIntegration, "qawc", integration_qawc, -1);
  rb_define_singleton_method(mIntegration, "qaws", integration_qaws, -1);
}
