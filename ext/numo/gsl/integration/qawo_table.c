#include "integration_internal.h"

#include <float.h>
#include <math.h>

static VALUE integration_cQAWOTable;

static int integration_qawo_table_double_equal(double left, double right) {
  double scale = fmax(fabs(left), fabs(right));

  return fabs(left - right) <= DBL_EPSILON * fmax(1.0, scale) * 4.0;
}

static void integration_qawo_table_free(void *data) {
  integration_qawo_table *table = data;

  if (table->table != NULL) {
    gsl_integration_qawo_table_free(table->table);
  }

  xfree(table);
}

static size_t integration_qawo_table_memsize(const void *data) {
  const integration_qawo_table *table = data;

  if (table == NULL) {
    return 0;
  }

  return sizeof(integration_qawo_table) +
         (table->table == NULL ? 0
                               : sizeof(gsl_integration_qawo_table) +
                                     25 * table->levels * sizeof(double));
}

static const rb_data_type_t integration_qawo_table_type = {
    .wrap_struct_name = "Numo::GSL::Integration::QAWOTable",
    .function =
        {
            .dmark = NULL,
            .dfree = integration_qawo_table_free,
            .dsize = integration_qawo_table_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE integration_qawo_table_allocate(VALUE klass) {
  integration_qawo_table *table;
  VALUE object = TypedData_Make_Struct(klass, integration_qawo_table,
                                       &integration_qawo_table_type, table);

  table->table = NULL;
  table->omega = 0.0;
  table->length = 0.0;
  table->sine = GSL_INTEG_COSINE;
  table->levels = 0;
  table->busy = 0;

  return object;
}

static VALUE integration_qawo_table_initialize(int argc, VALUE *argv,
                                               VALUE self) {
  VALUE options;
  VALUE omega_value;
  VALUE length_value;
  VALUE sine_value;
  VALUE levels_value;
  ID keywords[] = {rb_intern("omega"), rb_intern("length"), rb_intern("sine"),
                   rb_intern("levels")};
  VALUE keyword_values[4];
  double omega;
  double length;
  int sine;
  size_t levels;
  integration_qawo_table *table;

  rb_scan_args(argc, argv, "0:", &options);

  if (NIL_P(options)) {
    rb_raise(rb_eArgError, "missing keywords: :omega, :length, :sine, :levels");
  }

  rb_get_kwargs(options, keywords, 4, 0, keyword_values);
  omega_value = keyword_values[0];
  length_value = keyword_values[1];
  sine_value = keyword_values[2];
  levels_value = keyword_values[3];

  if (!rb_obj_is_kind_of(omega_value, rb_cNumeric)) {
    rb_raise(rb_eTypeError, "omega must be Numeric");
  }
  if (!rb_obj_is_kind_of(length_value, rb_cNumeric)) {
    rb_raise(rb_eTypeError, "length must be Numeric");
  }
  if (!RB_INTEGER_TYPE_P(sine_value)) {
    rb_raise(rb_eTypeError, "sine must be an Integer");
  }
  if (!RB_INTEGER_TYPE_P(levels_value)) {
    rb_raise(rb_eTypeError, "levels must be an Integer");
  }

  omega = NUM2DBL(omega_value);
  length = NUM2DBL(length_value);
  sine = NUM2INT(sine_value);

  if (length <= 0.0) {
    rb_raise(rb_eArgError, "length must be positive");
  }
  if (sine != GSL_INTEG_SINE && sine != GSL_INTEG_COSINE) {
    rb_raise(rb_eArgError, "sine must be SINE or COSINE");
  }
  if (RTEST(rb_funcall(levels_value, rb_intern("<="), 1, INT2FIX(0)))) {
    rb_raise(rb_eArgError, "levels must be positive");
  }

  levels = NUM2SIZET(levels_value);
  TypedData_Get_Struct(self, integration_qawo_table,
                       &integration_qawo_table_type, table);
  table->omega = omega;
  table->length = length;
  table->sine = (enum gsl_integration_qawo_enum)sine;
  table->levels = levels;
  table->table = gsl_integration_qawo_table_alloc(table->omega, table->length,
                                                  table->sine, table->levels);

  if (table->table == NULL) {
    rb_memerror();
  }

  return self;
}

static VALUE integration_qawo_table_omega(VALUE self) {
  integration_qawo_table *table;

  TypedData_Get_Struct(self, integration_qawo_table,
                       &integration_qawo_table_type, table);

  return DBL2NUM(table->omega);
}

static VALUE integration_qawo_table_length(VALUE self) {
  integration_qawo_table *table;

  TypedData_Get_Struct(self, integration_qawo_table,
                       &integration_qawo_table_type, table);

  return DBL2NUM(table->length);
}

static VALUE integration_qawo_table_sine(VALUE self) {
  integration_qawo_table *table;

  TypedData_Get_Struct(self, integration_qawo_table,
                       &integration_qawo_table_type, table);

  return INT2NUM(table->sine);
}

static VALUE integration_qawo_table_levels(VALUE self) {
  integration_qawo_table *table;

  TypedData_Get_Struct(self, integration_qawo_table,
                       &integration_qawo_table_type, table);

  return SIZET2NUM(table->levels);
}

static VALUE integration_qawo_table_closed_p(VALUE self) {
  integration_qawo_table *table;

  TypedData_Get_Struct(self, integration_qawo_table,
                       &integration_qawo_table_type, table);

  return table->table == NULL ? Qtrue : Qfalse;
}

static VALUE integration_qawo_table_close(VALUE self) {
  integration_qawo_table *table;

  TypedData_Get_Struct(self, integration_qawo_table,
                       &integration_qawo_table_type, table);

  if (table->busy) {
    rb_raise(rb_eRuntimeError, "table is already in use");
  }
  if (table->table != NULL) {
    gsl_integration_qawo_table_free(table->table);
    table->table = NULL;
  }

  return Qnil;
}

static VALUE integration_qawo_table_yield(VALUE table) {
  return rb_yield(table);
}

static VALUE integration_qawo_table_open(int argc, VALUE *argv, VALUE klass) {
  VALUE table;

  table = rb_funcallv_kw(klass, rb_intern("new"), argc, argv, RB_PASS_KEYWORDS);

  return rb_ensure(integration_qawo_table_yield, table,
                   integration_qawo_table_close, table);
}

void integration_qawo_table_use_begin(VALUE table_value, double omega,
                                      double length,
                                      enum gsl_integration_qawo_enum sine,
                                      size_t levels,
                                      integration_qawo_table_use *use) {
  integration_qawo_table *table;

  use->table = NULL;
  use->owner = NULL;

  if (table_value == Qundef) {
    use->table = gsl_integration_qawo_table_alloc(omega, length, sine, levels);
    if (use->table == NULL) {
      rb_memerror();
    }
    return;
  }

  if (!rb_obj_is_kind_of(table_value, integration_cQAWOTable)) {
    rb_raise(rb_eTypeError,
             "table must be a Numo::GSL::Integration::QAWOTable");
  }

  TypedData_Get_Struct(table_value, integration_qawo_table,
                       &integration_qawo_table_type, table);

  if (table->table == NULL) {
    rb_raise(rb_eRuntimeError, "table is closed");
  }
  if (table->busy) {
    rb_raise(rb_eRuntimeError, "table is already in use");
  }
  if (!integration_qawo_table_double_equal(table->omega, omega)) {
    rb_raise(rb_eArgError, "omega does not match QAWO table");
  }
  if (!integration_qawo_table_double_equal(table->length, length)) {
    rb_raise(rb_eArgError, "interval length does not match QAWO table");
  }
  if (table->sine != sine) {
    rb_raise(rb_eArgError, "sine does not match QAWO table");
  }

  table->busy = 1;
  use->table = table->table;
  use->owner = table;
}

VALUE integration_qawo_table_use_end(VALUE data) {
  integration_qawo_table_use *use = (integration_qawo_table_use *)data;

  if (use->owner != NULL) {
    use->owner->busy = 0;
  } else if (use->table != NULL) {
    gsl_integration_qawo_table_free(use->table);
  }

  use->table = NULL;
  use->owner = NULL;

  return Qnil;
}

void integration_qawo_table_init(VALUE integration_module) {
  integration_cQAWOTable =
      rb_define_class_under(integration_module, "QAWOTable", rb_cObject);
  rb_define_alloc_func(integration_cQAWOTable, integration_qawo_table_allocate);
  rb_define_method(integration_cQAWOTable, "initialize",
                   integration_qawo_table_initialize, -1);
  rb_define_method(integration_cQAWOTable, "omega",
                   integration_qawo_table_omega, 0);
  rb_define_method(integration_cQAWOTable, "length",
                   integration_qawo_table_length, 0);
  rb_define_method(integration_cQAWOTable, "sine", integration_qawo_table_sine,
                   0);
  rb_define_method(integration_cQAWOTable, "levels",
                   integration_qawo_table_levels, 0);
  rb_define_method(integration_cQAWOTable, "closed?",
                   integration_qawo_table_closed_p, 0);
  rb_define_method(integration_cQAWOTable, "close",
                   integration_qawo_table_close, 0);
  rb_define_singleton_method(integration_cQAWOTable, "open",
                             integration_qawo_table_open, -1);
}
