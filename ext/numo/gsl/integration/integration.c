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

static VALUE integration_qag(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  VALUE limit_value = Qundef;
  VALUE key_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"), rb_intern("limit"),
                   rb_intern("key")};
  VALUE keyword_values[4];
  double epsabs;
  double epsrel;
  size_t limit;
  int key;
  gsl_integration_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  double result;
  double error;
  int status;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);
  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 4, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
    key_value = keyword_values[3];
  }

  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 1000 : NUM2SIZET(limit_value);
  key = key_value == Qundef ? GSL_INTEG_GAUSS61 : NUM2INT(key_value);

  params.block = rb_block_proc();
  params.x = Qnil;
  params.state = 0;

  function.function = integration_callback;
  function.params = &params;

  workspace = gsl_integration_workspace_alloc(limit);

  status = gsl_integration_qag(&function, NUM2DBL(a), NUM2DBL(b), epsabs,
                               epsrel, limit, key, workspace, &result, &error);

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

  rb_define_const(mIntegration, "GAUSS15", INT2NUM(GSL_INTEG_GAUSS15));
  rb_define_const(mIntegration, "GAUSS21", INT2NUM(GSL_INTEG_GAUSS21));
  rb_define_const(mIntegration, "GAUSS31", INT2NUM(GSL_INTEG_GAUSS31));
  rb_define_const(mIntegration, "GAUSS41", INT2NUM(GSL_INTEG_GAUSS41));
  rb_define_const(mIntegration, "GAUSS51", INT2NUM(GSL_INTEG_GAUSS51));
  rb_define_const(mIntegration, "GAUSS61", INT2NUM(GSL_INTEG_GAUSS61));

  rb_define_singleton_method(mIntegration, "qag", integration_qag, -1);
}
