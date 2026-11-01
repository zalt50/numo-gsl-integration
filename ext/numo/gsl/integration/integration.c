#include <gsl/gsl_integration.h>
#include <ruby.h>

void Init_integration(void) {
  VALUE mNumo;
  VALUE mGSL;
  VALUE mIntegration;

  mNumo = rb_define_module("Numo");
  mGSL = rb_define_module_under(mNumo, "GSL");
  mIntegration = rb_define_module_under(mGSL, "Integration");

  (void)mIntegration;
}
