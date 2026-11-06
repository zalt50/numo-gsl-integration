#include "integration_internal.h"

static VALUE integration_cWorkspace;

static void integration_workspace_free(void *data) {
  integration_workspace *workspace = data;

  if (workspace->workspace != NULL) {
    gsl_integration_workspace_free(workspace->workspace);
  }

  xfree(workspace);
}

static size_t integration_workspace_memsize(const void *data) {
  return data == NULL ? 0 : sizeof(integration_workspace);
}

static const rb_data_type_t integration_workspace_type = {
    .wrap_struct_name = "Numo::GSL::Integration::Workspace",
    .function =
        {
            .dmark = NULL,
            .dfree = integration_workspace_free,
            .dsize = integration_workspace_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE integration_workspace_allocate(VALUE klass) {
  integration_workspace *workspace;
  VALUE object = TypedData_Make_Struct(klass, integration_workspace,
                                       &integration_workspace_type, workspace);

  workspace->workspace = NULL;
  workspace->limit = 0;
  workspace->busy = 0;

  return object;
}

static VALUE integration_workspace_initialize(int argc, VALUE *argv,
                                              VALUE self) {
  VALUE options;
  VALUE limit_value;
  ID keywords[] = {rb_intern("limit")};
  VALUE keyword_values[1];
  integration_workspace *workspace;

  rb_scan_args(argc, argv, "0:", &options);
  if (NIL_P(options)) {
    rb_raise(rb_eArgError, "missing keyword: :limit");
  }
  rb_get_kwargs(options, keywords, 1, 0, keyword_values);
  limit_value = keyword_values[0];

  if (!RB_INTEGER_TYPE_P(limit_value)) {
    rb_raise(rb_eTypeError, "limit must be an Integer");
  }
  if (RTEST(rb_funcall(limit_value, rb_intern("<="), 1, INT2FIX(0)))) {
    rb_raise(rb_eArgError, "limit must be positive");
  }

  TypedData_Get_Struct(self, integration_workspace, &integration_workspace_type,
                       workspace);
  workspace->limit = NUM2SIZET(limit_value);
  workspace->workspace = gsl_integration_workspace_alloc(workspace->limit);
  if (workspace->workspace == NULL) {
    rb_memerror();
  }

  return self;
}

static VALUE integration_workspace_limit(VALUE self) {
  integration_workspace *workspace;

  TypedData_Get_Struct(self, integration_workspace, &integration_workspace_type,
                       workspace);

  return SIZET2NUM(workspace->limit);
}

static VALUE integration_workspace_closed_p(VALUE self) {
  integration_workspace *workspace;

  TypedData_Get_Struct(self, integration_workspace, &integration_workspace_type,
                       workspace);

  return workspace->workspace == NULL ? Qtrue : Qfalse;
}

static VALUE integration_workspace_close(VALUE self) {
  integration_workspace *workspace;

  TypedData_Get_Struct(self, integration_workspace, &integration_workspace_type,
                       workspace);

  if (workspace->busy) {
    rb_raise(rb_eRuntimeError, "workspace is already in use");
  }
  if (workspace->workspace != NULL) {
    gsl_integration_workspace_free(workspace->workspace);
    workspace->workspace = NULL;
  }

  return Qnil;
}

static VALUE integration_workspace_yield(VALUE workspace) {
  return rb_yield(workspace);
}

static VALUE integration_workspace_open(int argc, VALUE *argv, VALUE klass) {
  VALUE workspace;

  workspace =
      rb_funcallv_kw(klass, rb_intern("new"), argc, argv, RB_PASS_KEYWORDS);

  return rb_ensure(integration_workspace_yield, workspace,
                   integration_workspace_close, workspace);
}

void integration_workspace_use_begin(VALUE workspace_value, size_t *limit,
                                     int limit_given,
                                     integration_workspace_use *use) {
  integration_workspace *workspace;

  use->workspace = NULL;
  use->owner = NULL;

  if (workspace_value == Qundef) {
    use->workspace = gsl_integration_workspace_alloc(*limit);
    if (use->workspace == NULL) {
      rb_memerror();
    }
    return;
  }

  if (!rb_obj_is_kind_of(workspace_value, integration_cWorkspace)) {
    rb_raise(rb_eTypeError,
             "workspace must be a Numo::GSL::Integration::Workspace");
  }

  TypedData_Get_Struct(workspace_value, integration_workspace,
                       &integration_workspace_type, workspace);
  if (workspace->workspace == NULL) {
    rb_raise(rb_eRuntimeError, "workspace is closed");
  }
  if (workspace->busy) {
    rb_raise(rb_eRuntimeError, "workspace is already in use");
  }
  if (limit_given && *limit > workspace->limit) {
    rb_raise(rb_eArgError, "limit exceeds workspace capacity");
  }
  if (!limit_given) {
    *limit = workspace->limit;
  }

  workspace->busy = 1;
  use->workspace = workspace->workspace;
  use->owner = workspace;
}

VALUE integration_workspace_use_end(VALUE data) {
  integration_workspace_use *use = (integration_workspace_use *)data;

  if (use->owner != NULL) {
    use->owner->busy = 0;
  } else if (use->workspace != NULL) {
    gsl_integration_workspace_free(use->workspace);
  }

  use->workspace = NULL;
  use->owner = NULL;

  return Qnil;
}

void integration_workspace_init(VALUE integration_module) {
  integration_cWorkspace =
      rb_define_class_under(integration_module, "Workspace", rb_cObject);
  rb_define_alloc_func(integration_cWorkspace, integration_workspace_allocate);
  rb_define_method(integration_cWorkspace, "initialize",
                   integration_workspace_initialize, -1);
  rb_define_method(integration_cWorkspace, "limit", integration_workspace_limit,
                   0);
  rb_define_method(integration_cWorkspace, "closed?",
                   integration_workspace_closed_p, 0);
  rb_define_method(integration_cWorkspace, "close", integration_workspace_close,
                   0);
  rb_define_singleton_method(integration_cWorkspace, "open",
                             integration_workspace_open, -1);
}
