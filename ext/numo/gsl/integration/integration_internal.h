#ifndef NUMO_GSL_INTEGRATION_INTERNAL_H
#define NUMO_GSL_INTEGRATION_INTERNAL_H

#include <gsl/gsl_integration.h>
#include <ruby.h>

typedef struct {
  VALUE block;
  VALUE x;
  VALUE exception;
  int state;
} integration_callback_params;

void integration_callback_params_init(integration_callback_params *params);
double integration_callback(double x, void *data);
void integration_raise_callback_exception(integration_callback_params *params);
VALUE integration_free_workspace(VALUE data);

VALUE integration_qag(int argc, VALUE *argv, VALUE self);
VALUE integration_qng(int argc, VALUE *argv, VALUE self);
VALUE integration_qags(int argc, VALUE *argv, VALUE self);
VALUE integration_qagp(int argc, VALUE *argv, VALUE self);
VALUE integration_qagi(int argc, VALUE *argv, VALUE self);
VALUE integration_qagiu(int argc, VALUE *argv, VALUE self);
VALUE integration_qagil(int argc, VALUE *argv, VALUE self);
VALUE integration_qawc(int argc, VALUE *argv, VALUE self);

#endif
