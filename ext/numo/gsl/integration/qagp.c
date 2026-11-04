#include "integration_internal.h"

#include <math.h>

typedef struct {
  VALUE points_array;
  gsl_function *function;
  double *points;
  size_t point_count;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  double *result;
  double *error;
} integration_qagp_params;

static void integration_validate_qagp_points(VALUE points) {
  long point_count;
  long index;
  double previous = 0.0;

  if (!RB_TYPE_P(points, T_ARRAY)) {
    rb_raise(integration_error_class(), "points must be an Array");
  }

  point_count = RARRAY_LEN(points);
  if (point_count < 2) {
    rb_raise(integration_error_class(),
             "points must contain at least two values");
  }

  for (index = 0; index < point_count; index++) {
    VALUE point = rb_ary_entry(points, index);
    double current;

    if (!rb_obj_is_kind_of(point, rb_cNumeric)) {
      rb_raise(integration_error_class(),
               "points must contain only numeric values");
    }

    current = NUM2DBL(point);
    if (!isfinite(current)) {
      rb_raise(integration_error_class(),
               "points must contain only finite values");
    }

    if (index > 0 && !(previous < current)) {
      rb_raise(integration_error_class(), "points must be strictly increasing");
    }

    previous = current;
  }
}

static VALUE integration_run_qagp(VALUE data) {
  integration_qagp_params *params = (integration_qagp_params *)data;
  size_t index;

  params->points = ALLOC_N(double, params->point_count);
  for (index = 0; index < params->point_count; index++) {
    params->points[index] =
        NUM2DBL(rb_ary_entry(params->points_array, (long)index));
  }
  params->workspace = gsl_integration_workspace_alloc(params->limit);
  (void)gsl_integration_qagp(params->function, params->points,
                             params->point_count, params->epsabs,
                             params->epsrel, params->limit, params->workspace,
                             params->result, params->error);
  return Qnil;
}

static VALUE integration_free_qagp_resources(VALUE data) {
  integration_qagp_params *params = (integration_qagp_params *)data;

  if (params->workspace != NULL) {
    gsl_integration_workspace_free(params->workspace);
  }
  if (params->points != NULL) {
    xfree(params->points);
  }
  return Qnil;
}

static VALUE integration_run_qagp_with_resources(VALUE data) {
  return rb_ensure(integration_run_qagp, data, integration_free_qagp_resources,
                   data);
}

VALUE integration_qagp(int argc, VALUE *argv, VALUE self) {
  VALUE points;
  VALUE options;
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_function function;
  integration_callback_params params;
  integration_qagp_params qagp_params;
  double result;
  double error;
  int state = 0;

  (void)self;
  rb_scan_args(argc, argv, "1:", &points, &options);

  integration_validate_qagp_points(points);
  integration_options_parse(options, &parsed_options, 0.0, 1e-8, 1000);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;
  integration_function_init(&function, &params);
  qagp_params.points_array = points;
  qagp_params.function = &function;
  qagp_params.points = NULL;
  qagp_params.point_count = (size_t)RARRAY_LEN(points);
  qagp_params.epsabs = epsabs;
  qagp_params.epsrel = epsrel;
  qagp_params.limit = limit;
  qagp_params.workspace = NULL;
  qagp_params.result = &result;
  qagp_params.error = &error;
  rb_protect(integration_run_qagp_with_resources, (VALUE)&qagp_params, &state);
  RB_GC_GUARD(points);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
