#include "integration_internal.h"

#include <float.h>
#include <math.h>

static VALUE integration_cQAWSTable;

static int integration_qaws_table_double_equal(double left, double right) {
  double scale = fmax(fabs(left), fabs(right));

  return fabs(left - right) <= DBL_EPSILON * fmax(1.0, scale) * 4.0;
}

static void integration_qaws_table_free(void *data) {
  integration_qaws_table *table = data;

  if (table->table != NULL) {
    gsl_integration_qaws_table_free(table->table);
  }

  xfree(table);
}

static size_t integration_qaws_table_memsize(const void *data) {
  const integration_qaws_table *table = data;

  if (table == NULL) {
    return 0;
  }

  return sizeof(integration_qaws_table) +
         (table->table == NULL ? 0 : sizeof(gsl_integration_qaws_table));
}

static const rb_data_type_t integration_qaws_table_type = {
    .wrap_struct_name = "Numo::GSL::Integration::QAWSTable",
    .function =
        {
            .dmark = NULL,
            .dfree = integration_qaws_table_free,
            .dsize = integration_qaws_table_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE integration_qaws_table_allocate(VALUE klass) {
  integration_qaws_table *table;
  VALUE object = TypedData_Make_Struct(klass, integration_qaws_table,
                                       &integration_qaws_table_type, table);

  table->table = NULL;
  table->alpha = 0.0;
  table->beta = 0.0;
  table->mu = 0;
  table->nu = 0;
  table->busy = 0;

  return object;
}

static VALUE integration_qaws_table_initialize(int argc, VALUE *argv,
                                               VALUE self) {
  VALUE options;
  VALUE alpha_value;
  VALUE beta_value;
  VALUE mu_value;
  VALUE nu_value;
  ID keywords[] = {rb_intern("alpha"), rb_intern("beta"), rb_intern("mu"),
                   rb_intern("nu")};
  VALUE keyword_values[4];
  double alpha;
  double beta;
  int mu;
  int nu;
  integration_qaws_table *table;

  rb_scan_args(argc, argv, "0:", &options);

  if (NIL_P(options)) {
    rb_raise(rb_eArgError, "missing keywords: :alpha, :beta, :mu, :nu");
  }

  rb_get_kwargs(options, keywords, 4, 0, keyword_values);
  alpha_value = keyword_values[0];
  beta_value = keyword_values[1];
  mu_value = keyword_values[2];
  nu_value = keyword_values[3];

  if (!rb_obj_is_kind_of(alpha_value, rb_cNumeric)) {
    rb_raise(rb_eTypeError, "alpha must be Numeric");
  }
  if (!rb_obj_is_kind_of(beta_value, rb_cNumeric)) {
    rb_raise(rb_eTypeError, "beta must be Numeric");
  }
  if (!RB_INTEGER_TYPE_P(mu_value)) {
    rb_raise(rb_eTypeError, "mu must be an Integer");
  }
  if (!RB_INTEGER_TYPE_P(nu_value)) {
    rb_raise(rb_eTypeError, "nu must be an Integer");
  }

  alpha = NUM2DBL(alpha_value);
  beta = NUM2DBL(beta_value);
  mu = NUM2INT(mu_value);
  nu = NUM2INT(nu_value);

  if (!(alpha > -1.0)) {
    rb_raise(rb_eArgError, "alpha must be greater than -1");
  }
  if (!(beta > -1.0)) {
    rb_raise(rb_eArgError, "beta must be greater than -1");
  }
  if (mu != 0 && mu != 1) {
    rb_raise(rb_eArgError, "mu must be 0 or 1");
  }
  if (nu != 0 && nu != 1) {
    rb_raise(rb_eArgError, "nu must be 0 or 1");
  }

  TypedData_Get_Struct(self, integration_qaws_table,
                       &integration_qaws_table_type, table);
  table->alpha = alpha;
  table->beta = beta;
  table->mu = mu;
  table->nu = nu;
  table->table =
      gsl_integration_qaws_table_alloc(alpha, beta, table->mu, table->nu);

  if (table->table == NULL) {
    rb_memerror();
  }

  return self;
}

static VALUE integration_qaws_table_alpha(VALUE self) {
  integration_qaws_table *table;

  TypedData_Get_Struct(self, integration_qaws_table,
                       &integration_qaws_table_type, table);

  return DBL2NUM(table->alpha);
}

static VALUE integration_qaws_table_beta(VALUE self) {
  integration_qaws_table *table;

  TypedData_Get_Struct(self, integration_qaws_table,
                       &integration_qaws_table_type, table);

  return DBL2NUM(table->beta);
}

static VALUE integration_qaws_table_mu(VALUE self) {
  integration_qaws_table *table;

  TypedData_Get_Struct(self, integration_qaws_table,
                       &integration_qaws_table_type, table);

  return INT2NUM(table->mu);
}

static VALUE integration_qaws_table_nu(VALUE self) {
  integration_qaws_table *table;

  TypedData_Get_Struct(self, integration_qaws_table,
                       &integration_qaws_table_type, table);

  return INT2NUM(table->nu);
}

static VALUE integration_qaws_table_closed_p(VALUE self) {
  integration_qaws_table *table;

  TypedData_Get_Struct(self, integration_qaws_table,
                       &integration_qaws_table_type, table);

  return table->table == NULL ? Qtrue : Qfalse;
}

static VALUE integration_qaws_table_close(VALUE self) {
  integration_qaws_table *table;

  TypedData_Get_Struct(self, integration_qaws_table,
                       &integration_qaws_table_type, table);

  if (table->busy) {
    rb_raise(rb_eRuntimeError, "table is already in use");
  }
  if (table->table != NULL) {
    gsl_integration_qaws_table_free(table->table);
    table->table = NULL;
  }

  return Qnil;
}

static VALUE integration_qaws_table_yield(VALUE table) {
  return rb_yield(table);
}

static VALUE integration_qaws_table_open(int argc, VALUE *argv, VALUE klass) {
  VALUE table;

  table = rb_funcallv_kw(klass, rb_intern("new"), argc, argv, RB_PASS_KEYWORDS);

  return rb_ensure(integration_qaws_table_yield, table,
                   integration_qaws_table_close, table);
}

void integration_qaws_table_use_begin(VALUE table_value, double alpha,
                                      double beta, int mu, int nu,
                                      integration_qaws_table_use *use) {
  integration_qaws_table *table;

  use->table = NULL;
  use->owner = NULL;

  if (table_value == Qundef) {
    use->table = gsl_integration_qaws_table_alloc(alpha, beta, mu, nu);
    if (use->table == NULL) {
      rb_memerror();
    }
    return;
  }

  if (!rb_obj_is_kind_of(table_value, integration_cQAWSTable)) {
    rb_raise(rb_eTypeError,
             "table must be a Numo::GSL::Integration::QAWSTable");
  }

  TypedData_Get_Struct(table_value, integration_qaws_table,
                       &integration_qaws_table_type, table);

  if (table->table == NULL) {
    rb_raise(rb_eRuntimeError, "table is closed");
  }
  if (table->busy) {
    rb_raise(rb_eRuntimeError, "table is already in use");
  }
  if (!integration_qaws_table_double_equal(table->alpha, alpha)) {
    rb_raise(rb_eArgError, "alpha does not match QAWS table");
  }
  if (!integration_qaws_table_double_equal(table->beta, beta)) {
    rb_raise(rb_eArgError, "beta does not match QAWS table");
  }
  if (table->mu != mu) {
    rb_raise(rb_eArgError, "mu does not match QAWS table");
  }
  if (table->nu != nu) {
    rb_raise(rb_eArgError, "nu does not match QAWS table");
  }

  table->busy = 1;
  use->table = table->table;
  use->owner = table;
}

VALUE integration_qaws_table_use_end(VALUE data) {
  integration_qaws_table_use *use = (integration_qaws_table_use *)data;

  if (use->owner != NULL) {
    use->owner->busy = 0;
  } else if (use->table != NULL) {
    gsl_integration_qaws_table_free(use->table);
  }

  use->table = NULL;
  use->owner = NULL;

  return Qnil;
}

void integration_qaws_table_init(VALUE integration_module) {
  integration_cQAWSTable =
      rb_define_class_under(integration_module, "QAWSTable", rb_cObject);
  rb_define_alloc_func(integration_cQAWSTable, integration_qaws_table_allocate);
  rb_define_method(integration_cQAWSTable, "initialize",
                   integration_qaws_table_initialize, -1);
  rb_define_method(integration_cQAWSTable, "alpha",
                   integration_qaws_table_alpha, 0);
  rb_define_method(integration_cQAWSTable, "beta", integration_qaws_table_beta,
                   0);
  rb_define_method(integration_cQAWSTable, "mu", integration_qaws_table_mu, 0);
  rb_define_method(integration_cQAWSTable, "nu", integration_qaws_table_nu, 0);
  rb_define_method(integration_cQAWSTable, "closed?",
                   integration_qaws_table_closed_p, 0);
  rb_define_method(integration_cQAWSTable, "close",
                   integration_qaws_table_close, 0);
  rb_define_singleton_method(integration_cQAWSTable, "open",
                             integration_qaws_table_open, -1);
}
