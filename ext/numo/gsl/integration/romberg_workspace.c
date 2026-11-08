#include "integration_internal.h"

static VALUE integration_cRombergWorkspace;

static void integration_romberg_workspace_free(void *data) {
  integration_romberg_workspace *workspace = data;

  if (workspace->workspace != NULL) {
    gsl_integration_romberg_free(workspace->workspace);
  }

  xfree(workspace);
}

static size_t integration_romberg_workspace_memsize(const void *data) {
  return data == NULL ? 0 : sizeof(integration_romberg_workspace);
}

static const rb_data_type_t integration_romberg_workspace_type = {
    .wrap_struct_name = "Numo::GSL::Integration::RombergWorkspace",
    .function =
        {
            .dmark = NULL,
            .dfree = integration_romberg_workspace_free,
            .dsize = integration_romberg_workspace_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE integration_romberg_workspace_allocate(VALUE klass) {
  integration_romberg_workspace *workspace;
  VALUE object =
      TypedData_Make_Struct(klass, integration_romberg_workspace,
                            &integration_romberg_workspace_type, workspace);

  workspace->workspace = NULL;
  workspace->limit = 0;
  workspace->busy = 0;

  return object;
}

static VALUE integration_romberg_workspace_initialize(int argc, VALUE *argv,
                                                      VALUE self) {
  VALUE options;
  VALUE limit_value;
  ID keywords[] = {rb_intern("limit")};
  VALUE keyword_values[1];
  size_t limit;
  gsl_integration_romberg_workspace *native_workspace;
  integration_romberg_workspace *workspace;

  rb_scan_args(argc, argv, "0:", &options);
  if (NIL_P(options)) {
    rb_raise(rb_eArgError, "missing keyword: :limit");
  }

  rb_get_kwargs(options, keywords, 1, 0, keyword_values);
  limit_value = keyword_values[0];
  if (!RB_INTEGER_TYPE_P(limit_value)) {
    rb_raise(rb_eTypeError, "limit must be an Integer");
  }
  if (RTEST(rb_funcall(limit_value, rb_intern("<"), 1, INT2FIX(1))) ||
      RTEST(rb_funcall(limit_value, rb_intern(">"), 1, INT2FIX(30)))) {
    rb_raise(rb_eArgError, "limit must be between 1 and 30");
  }

  TypedData_Get_Struct(self, integration_romberg_workspace,
                       &integration_romberg_workspace_type, workspace);
  if (workspace->workspace != NULL) {
    rb_raise(rb_eRuntimeError, "workspace is already initialized");
  }

  limit = NUM2SIZET(limit_value);
  native_workspace = gsl_integration_romberg_alloc(limit);
  if (native_workspace == NULL) {
    rb_memerror();
  }

  workspace->workspace = native_workspace;
  workspace->limit = limit;

  return self;
}

static VALUE integration_romberg_workspace_limit(VALUE self) {
  integration_romberg_workspace *workspace;

  TypedData_Get_Struct(self, integration_romberg_workspace,
                       &integration_romberg_workspace_type, workspace);

  return SIZET2NUM(workspace->limit);
}

static VALUE integration_romberg_workspace_closed_p(VALUE self) {
  integration_romberg_workspace *workspace;

  TypedData_Get_Struct(self, integration_romberg_workspace,
                       &integration_romberg_workspace_type, workspace);

  return workspace->workspace == NULL ? Qtrue : Qfalse;
}

static VALUE integration_romberg_workspace_close(VALUE self) {
  integration_romberg_workspace *workspace;

  TypedData_Get_Struct(self, integration_romberg_workspace,
                       &integration_romberg_workspace_type, workspace);
  if (workspace->busy) {
    rb_raise(rb_eRuntimeError, "workspace is already in use");
  }
  if (workspace->workspace != NULL) {
    gsl_integration_romberg_free(workspace->workspace);
    workspace->workspace = NULL;
  }

  return Qnil;
}

static VALUE integration_romberg_workspace_yield(VALUE workspace) {
  return rb_yield(workspace);
}

static VALUE integration_romberg_workspace_open(int argc, VALUE *argv,
                                                VALUE klass) {
  VALUE workspace;

  workspace =
      rb_funcallv_kw(klass, rb_intern("new"), argc, argv, RB_PASS_KEYWORDS);

  return rb_ensure(integration_romberg_workspace_yield, workspace,
                   integration_romberg_workspace_close, workspace);
}

static integration_romberg_workspace *
integration_romberg_workspace_get(VALUE workspace_value) {
  integration_romberg_workspace *workspace;

  if (!rb_obj_is_kind_of(workspace_value, integration_cRombergWorkspace)) {
    rb_raise(rb_eTypeError,
             "workspace must be a Numo::GSL::Integration::RombergWorkspace");
  }

  TypedData_Get_Struct(workspace_value, integration_romberg_workspace,
                       &integration_romberg_workspace_type, workspace);
  if (workspace->workspace == NULL) {
    rb_raise(rb_eRuntimeError, "workspace is closed");
  }
  if (workspace->busy) {
    rb_raise(rb_eRuntimeError, "workspace is already in use");
  }

  return workspace;
}

void integration_romberg_workspace_use_begin(
    VALUE workspace_value, size_t *limit, int limit_given,
    integration_romberg_workspace_use *use) {
  integration_romberg_workspace *workspace;

  use->workspace = NULL;
  use->owner = NULL;
  if (workspace_value == Qundef) {
    use->workspace = gsl_integration_romberg_alloc(*limit);
    if (use->workspace == NULL) {
      rb_memerror();
    }
    return;
  }

  workspace = integration_romberg_workspace_get(workspace_value);
  if (limit_given && *limit != workspace->limit) {
    rb_raise(rb_eArgError, "limit must match workspace capacity");
  }
  if (!limit_given) {
    *limit = workspace->limit;
  }

  workspace->busy = 1;
  use->workspace = workspace->workspace;
  use->owner = workspace;
}

VALUE integration_romberg_workspace_use_end(VALUE data) {
  integration_romberg_workspace_use *use =
      (integration_romberg_workspace_use *)data;

  if (use->owner != NULL) {
    use->owner->busy = 0;
  } else if (use->workspace != NULL) {
    gsl_integration_romberg_free(use->workspace);
  }

  use->workspace = NULL;
  use->owner = NULL;

  return Qnil;
}

void integration_romberg_workspace_init(VALUE integration_module) {
  integration_cRombergWorkspace =
      rb_define_class_under(integration_module, "RombergWorkspace", rb_cObject);
  rb_define_alloc_func(integration_cRombergWorkspace,
                       integration_romberg_workspace_allocate);
  rb_define_method(integration_cRombergWorkspace, "initialize",
                   integration_romberg_workspace_initialize, -1);
  rb_define_method(integration_cRombergWorkspace, "limit",
                   integration_romberg_workspace_limit, 0);
  rb_define_method(integration_cRombergWorkspace, "closed?",
                   integration_romberg_workspace_closed_p, 0);
  rb_define_method(integration_cRombergWorkspace, "close",
                   integration_romberg_workspace_close, 0);
  rb_define_singleton_method(integration_cRombergWorkspace, "open",
                             integration_romberg_workspace_open, -1);
}
