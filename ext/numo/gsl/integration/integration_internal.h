#ifndef NUMO_GSL_INTEGRATION_INTERNAL_H
#define NUMO_GSL_INTEGRATION_INTERNAL_H

#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_math.h>
#include <ruby.h>

typedef struct {
  double epsabs;
  double epsrel;
  size_t limit;
} integration_options;

void integration_options_parse(VALUE options, integration_options *parsed,
                               double default_epsabs, double default_epsrel,
                               size_t default_limit);
void integration_options_parse_with_workspace(
    VALUE options, integration_options *parsed, double default_epsabs,
    double default_epsrel, size_t default_limit, VALUE *workspace_value,
    int *limit_given);

typedef struct {
  gsl_integration_workspace *workspace;
  size_t limit;
  int busy;
} integration_workspace;

typedef struct {
  gsl_integration_workspace *workspace;
  integration_workspace *owner;
} integration_workspace_use;

void integration_workspace_init(VALUE integration_module);
size_t integration_workspace_capacity(VALUE workspace_value, const char *name);
void integration_workspace_use_begin_named(VALUE workspace_value, size_t *limit,
                                           int limit_given,
                                           integration_workspace_use *use,
                                           const char *name);
void integration_workspace_use_begin(VALUE workspace_value, size_t *limit,
                                     int limit_given,
                                     integration_workspace_use *use);
VALUE integration_workspace_use_end(VALUE data);

typedef struct {
  gsl_integration_cquad_workspace *workspace;
  size_t limit;
  int busy;
} integration_cquad_workspace;
typedef struct {
  gsl_integration_cquad_workspace *workspace;
  integration_cquad_workspace *owner;
} integration_cquad_workspace_use;
void integration_cquad_workspace_init(VALUE integration_module);
void integration_cquad_workspace_use_begin(
    VALUE workspace_value, size_t *limit, int limit_given,
    integration_cquad_workspace_use *use);
VALUE integration_cquad_workspace_use_end(VALUE data);
typedef struct {
  gsl_integration_romberg_workspace *workspace;
  size_t limit;
  int busy;
} integration_romberg_workspace;
typedef struct {
  gsl_integration_romberg_workspace *workspace;
  integration_romberg_workspace *owner;
} integration_romberg_workspace_use;
void integration_romberg_workspace_init(VALUE integration_module);
void integration_romberg_workspace_use_begin(
    VALUE workspace_value, size_t *limit, int limit_given,
    integration_romberg_workspace_use *use);
VALUE integration_romberg_workspace_use_end(VALUE data);
typedef struct {
  gsl_integration_glfixed_table *table;
  size_t order;
  int busy;
} integration_glfixed_table;
typedef struct {
  gsl_integration_glfixed_table *table;
  integration_glfixed_table *owner;
} integration_glfixed_table_use;
void integration_glfixed_table_init(VALUE integration_module);
void integration_glfixed_table_use_begin(VALUE table_value, size_t *order,
                                         int order_given,
                                         integration_glfixed_table_use *use);
VALUE integration_glfixed_table_use_end(VALUE data);
typedef struct {
  gsl_integration_qawo_table *table;
  double omega;
  double length;
  enum gsl_integration_qawo_enum sine;
  size_t levels;
  int busy;
} integration_qawo_table;

typedef struct {
  gsl_integration_qawo_table *table;
  integration_qawo_table *owner;
} integration_qawo_table_use;

void integration_qawo_table_init(VALUE integration_module);
void integration_qawo_table_use_begin(VALUE table_value, double omega,
                                      double length,
                                      enum gsl_integration_qawo_enum sine,
                                      size_t levels,
                                      integration_qawo_table_use *use);
VALUE integration_qawo_table_use_end(VALUE data);
void integration_qawf_table_init(VALUE integration_module);
void integration_qawf_table_use_begin(VALUE table_value, double omega,
                                      enum gsl_integration_qawo_enum sine,
                                      size_t levels,
                                      integration_qawo_table_use *use);

typedef struct {
  gsl_integration_qaws_table *table;
  double alpha;
  double beta;
  int mu;
  int nu;
  int busy;
} integration_qaws_table;

typedef struct {
  gsl_integration_qaws_table *table;
  integration_qaws_table *owner;
} integration_qaws_table_use;

void integration_qaws_table_init(VALUE integration_module);
void integration_qaws_table_use_begin(VALUE table_value, double alpha,
                                      double beta, int mu, int nu,
                                      integration_qaws_table_use *use);
VALUE integration_qaws_table_use_end(VALUE data);

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

void integration_fixed_init(void);

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
VALUE integration_fixed(int argc, VALUE *argv, VALUE self);
VALUE integration_lebedev(VALUE self, VALUE n_value);

#endif
