#ifndef NUMO_GSL_INTEGRATION_INTERNAL_H
#define NUMO_GSL_INTEGRATION_INTERNAL_H
#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <ruby.h>
typedef struct {
  double epsabs;
  double epsrel;
  size_t limit;
} integration_options;
void integration_options_parse(VALUE options, integration_options *parsed,
                               double default_epsabs, double default_epsrel,
                               size_t default_limit);
typedef struct {
  VALUE block;
  VALUE x;
  VALUE exception;
  int state;
} integration_callback_params;
void integration_callback_params_init(integration_callback_params *params);
void integration_function_init(gsl_function *function,
                               integration_callback_params *params);
double integration_callback(double x, void *data);
void integration_raise_callback_exception(integration_callback_params *params);
void integration_finish_callback(integration_callback_params *params,
                                 int state);
VALUE integration_free_workspace(VALUE data);
VALUE integration_free_cquad_workspace(VALUE data);
VALUE integration_free_romberg_workspace(VALUE data);
VALUE integration_free_glfixed_table(VALUE data);
VALUE integration_qag(int argc, VALUE *argv, VALUE self);
VALUE integration_qng(int argc, VALUE *argv, VALUE self);
VALUE integration_qags(int argc, VALUE *argv, VALUE self);
VALUE integration_qagp(int argc, VALUE *argv, VALUE self);
VALUE integration_qagi(int argc, VALUE *argv, VALUE self);
VALUE integration_qagiu(int argc, VALUE *argv, VALUE self);
VALUE integration_qagil(int argc, VALUE *argv, VALUE self);
VALUE integration_qawc(int argc, VALUE *argv, VALUE self);
VALUE integration_qaws(int argc, VALUE *argv, VALUE self);
VALUE integration_qawo(int argc, VALUE *argv, VALUE self);
VALUE integration_qawf(int argc, VALUE *argv, VALUE self);
VALUE integration_cquad(int argc, VALUE *argv, VALUE self);
VALUE integration_romberg(int argc, VALUE *argv, VALUE self);
VALUE integration_glfixed(int argc, VALUE *argv, VALUE self);
#endif
