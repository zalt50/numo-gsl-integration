#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <ruby.h>

static VALUE eIntegrationError;

typedef struct {
  VALUE block;
  VALUE x;
  int state;
} integration_callback_params;

static VALUE integration_call_block(VALUE data) {
  integration_callback_params *params = (integration_callback_params *)data;
  VALUE value;

  value = rb_funcall(params->block, rb_intern("call"), 1, params->x);

  return rb_Float(value);
}

static double integration_callback(double x, void *data) {
  integration_callback_params *params = data;
  VALUE value;

  if (params->state != 0) {
    return 0.0;
  }

  params->x = DBL2NUM(x);
  value = rb_protect(integration_call_block, (VALUE)params, &params->state);

  return params->state == 0 ? NUM2DBL(value) : 0.0;
}

static VALUE integration_qag(VALUE self, VALUE a, VALUE b) {
  const size_t limit = 1000;
  gsl_integration_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  double result;
  double error;
  int status;

  (void)self;

  params.block = rb_block_proc();
  params.x = Qnil;
  params.state = 0;

  function.function = integration_callback;
  function.params = &params;

  workspace = gsl_integration_workspace_alloc(limit);

  status =
      gsl_integration_qag(&function, NUM2DBL(a), NUM2DBL(b), 0.0, 1e-8, limit,
                          GSL_INTEG_GAUSS61, workspace, &result, &error);

  gsl_integration_workspace_free(workspace);

  RB_GC_GUARD(params.block);

  if (params.state != 0) {
    rb_jump_tag(params.state);
  }

  if (status != GSL_SUCCESS) {
    rb_raise(eIntegrationError, "%s", gsl_strerror(status));
  }

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}

void Init_integration(void) {
  VALUE mNumo;
  VALUE mGSL;
  VALUE mIntegration;

  mNumo = rb_define_module("Numo");
  mGSL = rb_define_module_under(mNumo, "GSL");
  mIntegration = rb_define_module_under(mGSL, "Integration");

  eIntegrationError = rb_const_get(mIntegration, rb_intern("Error"));

  gsl_set_error_handler_off();

  rb_define_singleton_method(mIntegration, "qag", integration_qag, 2);
}
