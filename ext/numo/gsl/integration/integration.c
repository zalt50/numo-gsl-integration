#include <gsl/gsl_integration.h>
#include <ruby.h>

static double integration_callback(double x, void *params) {
  VALUE block = *(VALUE *)params;

  return NUM2DBL(rb_funcall(block, rb_intern("call"), 1, DBL2NUM(x)));
}

static VALUE integration_qag(VALUE self, VALUE a, VALUE b) {
  const size_t limit = 1000;
  gsl_integration_workspace *workspace;
  gsl_function function;
  VALUE block;
  double result;
  double error;

  (void)self;

  block = rb_block_proc();

  function.function = integration_callback;
  function.params = &block;

  workspace = gsl_integration_workspace_alloc(limit);

  gsl_integration_qag(&function, NUM2DBL(a), NUM2DBL(b), 0.0, 1e-8, limit,
                      GSL_INTEG_GAUSS61, workspace, &result, &error);

  gsl_integration_workspace_free(workspace);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}

void Init_integration(void) {
  VALUE mNumo;
  VALUE mGSL;
  VALUE mIntegration;

  mNumo = rb_define_module("Numo");
  mGSL = rb_define_module_under(mNumo, "GSL");
  mIntegration = rb_define_module_under(mGSL, "Integration");

  rb_define_singleton_method(mIntegration, "qag", integration_qag, 2);
}
