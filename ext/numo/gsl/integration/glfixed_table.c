#include "integration_internal.h"

static VALUE integration_cGLFixedTable;

static void integration_glfixed_table_free(void *data) {
  integration_glfixed_table *table = data;

  if (table->table != NULL) {
    gsl_integration_glfixed_table_free(table->table);
  }

  xfree(table);
}

static size_t integration_glfixed_table_memsize(const void *data) {
  return data == NULL ? 0 : sizeof(integration_glfixed_table);
}

static const rb_data_type_t integration_glfixed_table_type = {
    .wrap_struct_name = "Numo::GSL::Integration::GLFixedTable",
    .function =
        {
            .dmark = NULL,
            .dfree = integration_glfixed_table_free,
            .dsize = integration_glfixed_table_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE integration_glfixed_table_allocate(VALUE klass) {
  integration_glfixed_table *table;
  VALUE object = TypedData_Make_Struct(klass, integration_glfixed_table,
                                       &integration_glfixed_table_type, table);

  table->table = NULL;
  table->order = 0;
  table->busy = 0;

  return object;
}

static VALUE integration_glfixed_table_initialize(int argc, VALUE *argv,
                                                  VALUE self) {
  VALUE options;
  VALUE order_value;
  ID keywords[] = {rb_intern("order")};
  VALUE keyword_values[1];
  size_t order;
  gsl_integration_glfixed_table *native_table;
  integration_glfixed_table *table;

  rb_scan_args(argc, argv, "0:", &options);
  if (NIL_P(options)) {
    rb_raise(rb_eArgError, "missing keyword: :order");
  }

  rb_get_kwargs(options, keywords, 1, 0, keyword_values);
  order_value = keyword_values[0];
  if (!RB_INTEGER_TYPE_P(order_value)) {
    rb_raise(rb_eTypeError, "order must be an Integer");
  }
  if (RTEST(rb_funcall(order_value, rb_intern("<="), 1, INT2FIX(0)))) {
    rb_raise(rb_eArgError, "order must be positive");
  }

  TypedData_Get_Struct(self, integration_glfixed_table,
                       &integration_glfixed_table_type, table);
  if (table->table != NULL) {
    rb_raise(rb_eRuntimeError, "table is already initialized");
  }

  order = NUM2SIZET(order_value);
  native_table = gsl_integration_glfixed_table_alloc(order);
  if (native_table == NULL) {
    rb_memerror();
  }

  table->table = native_table;
  table->order = order;

  return self;
}

static VALUE integration_glfixed_table_order(VALUE self) {
  integration_glfixed_table *table;

  TypedData_Get_Struct(self, integration_glfixed_table,
                       &integration_glfixed_table_type, table);

  return SIZET2NUM(table->order);
}

static VALUE integration_glfixed_table_closed_p(VALUE self) {
  integration_glfixed_table *table;

  TypedData_Get_Struct(self, integration_glfixed_table,
                       &integration_glfixed_table_type, table);

  return table->table == NULL ? Qtrue : Qfalse;
}

static VALUE integration_glfixed_table_close(VALUE self) {
  integration_glfixed_table *table;

  TypedData_Get_Struct(self, integration_glfixed_table,
                       &integration_glfixed_table_type, table);
  if (table->busy) {
    rb_raise(rb_eRuntimeError, "table is already in use");
  }
  if (table->table != NULL) {
    gsl_integration_glfixed_table_free(table->table);
    table->table = NULL;
  }

  return Qnil;
}

static VALUE integration_glfixed_table_yield(VALUE table) {
  return rb_yield(table);
}

static VALUE integration_glfixed_table_open(int argc, VALUE *argv,
                                            VALUE klass) {
  VALUE table;

  table = rb_funcallv_kw(klass, rb_intern("new"), argc, argv, RB_PASS_KEYWORDS);

  return rb_ensure(integration_glfixed_table_yield, table,
                   integration_glfixed_table_close, table);
}

static integration_glfixed_table *
integration_glfixed_table_get(VALUE table_value) {
  integration_glfixed_table *table;

  if (!rb_obj_is_kind_of(table_value, integration_cGLFixedTable)) {
    rb_raise(rb_eTypeError,
             "table must be a Numo::GSL::Integration::GLFixedTable");
  }

  TypedData_Get_Struct(table_value, integration_glfixed_table,
                       &integration_glfixed_table_type, table);
  if (table->table == NULL) {
    rb_raise(rb_eRuntimeError, "table is closed");
  }
  if (table->busy) {
    rb_raise(rb_eRuntimeError, "table is already in use");
  }

  return table;
}

void integration_glfixed_table_use_begin(VALUE table_value, size_t *order,
                                         int order_given,
                                         integration_glfixed_table_use *use) {
  integration_glfixed_table *table;

  use->table = NULL;
  use->owner = NULL;
  if (table_value == Qundef) {
    use->table = gsl_integration_glfixed_table_alloc(*order);
    if (use->table == NULL) {
      rb_memerror();
    }
    return;
  }

  table = integration_glfixed_table_get(table_value);
  if (order_given && *order != table->order) {
    rb_raise(rb_eArgError, "order does not match GLFixed table");
  }
  if (!order_given) {
    *order = table->order;
  }

  table->busy = 1;
  use->table = table->table;
  use->owner = table;
}

VALUE integration_glfixed_table_use_end(VALUE data) {
  integration_glfixed_table_use *use = (integration_glfixed_table_use *)data;

  if (use->owner != NULL) {
    use->owner->busy = 0;
  } else if (use->table != NULL) {
    gsl_integration_glfixed_table_free(use->table);
  }

  use->table = NULL;
  use->owner = NULL;

  return Qnil;
}

void integration_glfixed_table_init(VALUE integration_module) {
  integration_cGLFixedTable =
      rb_define_class_under(integration_module, "GLFixedTable", rb_cObject);
  rb_define_alloc_func(integration_cGLFixedTable,
                       integration_glfixed_table_allocate);
  rb_define_method(integration_cGLFixedTable, "initialize",
                   integration_glfixed_table_initialize, -1);
  rb_define_method(integration_cGLFixedTable, "order",
                   integration_glfixed_table_order, 0);
  rb_define_method(integration_cGLFixedTable, "closed?",
                   integration_glfixed_table_closed_p, 0);
  rb_define_method(integration_cGLFixedTable, "close",
                   integration_glfixed_table_close, 0);
  rb_define_singleton_method(integration_cGLFixedTable, "open",
                             integration_glfixed_table_open, -1);
}
