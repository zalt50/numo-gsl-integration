#include "integration_internal.h"

typedef struct {
  gsl_integration_lebedev_workspace *workspace;
  size_t n;
  int busy;
} integration_lebedev_workspace;

typedef struct {
  gsl_integration_lebedev_workspace *workspace;
  integration_lebedev_workspace *owner;
} integration_lebedev_workspace_use;

typedef struct {
  VALUE block;
  VALUE x;
  VALUE y;
  VALUE z;
  VALUE exception;
  gsl_integration_lebedev_workspace *workspace;
  double result;
  int state;
} integration_lebedev_params;

static VALUE integration_cLebedevWorkspace;
static ID integration_lebedev_id_n;
static ID integration_lebedev_id_workspace;
static ID integration_lebedev_id_less_than_or_equal;

static size_t integration_lebedev_node_count(VALUE n_value) {
  if (!RB_INTEGER_TYPE_P(n_value)) {
    rb_raise(rb_eTypeError, "n must be an Integer");
  }
  if (RTEST(rb_funcall(n_value, integration_lebedev_id_less_than_or_equal, 1,
                       INT2FIX(0)))) {
    rb_raise(rb_eArgError, "n must be positive");
  }
  return NUM2SIZET(n_value);
}

static void integration_lebedev_workspace_free(void *data) {
  integration_lebedev_workspace *workspace = data;

  if (workspace->workspace != NULL) {
    gsl_integration_lebedev_free(workspace->workspace);
  }
  xfree(workspace);
}

static size_t integration_lebedev_workspace_memsize(const void *data) {
  return data == NULL ? 0 : sizeof(integration_lebedev_workspace);
}

static const rb_data_type_t integration_lebedev_workspace_type = {
    .wrap_struct_name = "Numo::GSL::Integration::LebedevWorkspace",
    .function =
        {
            .dmark = NULL,
            .dfree = integration_lebedev_workspace_free,
            .dsize = integration_lebedev_workspace_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE integration_lebedev_workspace_allocate(VALUE klass) {
  integration_lebedev_workspace *workspace;
  VALUE object =
      TypedData_Make_Struct(klass, integration_lebedev_workspace,
                            &integration_lebedev_workspace_type, workspace);

  workspace->workspace = NULL;
  workspace->n = 0;
  workspace->busy = 0;
  return object;
}

static VALUE integration_lebedev_workspace_initialize(int argc, VALUE *argv,
                                                      VALUE self) {
  VALUE options;
  ID keywords[] = {integration_lebedev_id_n};
  VALUE keyword_values[1];
  gsl_integration_lebedev_workspace *native_workspace;
  integration_lebedev_workspace *workspace;
  size_t n;

  rb_scan_args(argc, argv, "0:", &options);
  if (NIL_P(options)) {
    rb_raise(rb_eArgError, "missing keyword: :n");
  }
  rb_get_kwargs(options, keywords, 1, 0, keyword_values);
  n = integration_lebedev_node_count(keyword_values[0]);

  TypedData_Get_Struct(self, integration_lebedev_workspace,
                       &integration_lebedev_workspace_type, workspace);
  if (workspace->workspace != NULL) {
    rb_raise(rb_eRuntimeError, "workspace is already initialized");
  }

  native_workspace = gsl_integration_lebedev_alloc(n);
  if (native_workspace == NULL) {
    rb_memerror();
  }
  workspace->workspace = native_workspace;
  workspace->n = n;
  return self;
}

static VALUE integration_lebedev_workspace_n(VALUE self) {
  integration_lebedev_workspace *workspace;

  TypedData_Get_Struct(self, integration_lebedev_workspace,
                       &integration_lebedev_workspace_type, workspace);
  return SIZET2NUM(workspace->n);
}

static VALUE integration_lebedev_workspace_closed_p(VALUE self) {
  integration_lebedev_workspace *workspace;

  TypedData_Get_Struct(self, integration_lebedev_workspace,
                       &integration_lebedev_workspace_type, workspace);
  return workspace->workspace == NULL ? Qtrue : Qfalse;
}

static VALUE integration_lebedev_workspace_close(VALUE self) {
  integration_lebedev_workspace *workspace;

  TypedData_Get_Struct(self, integration_lebedev_workspace,
                       &integration_lebedev_workspace_type, workspace);
  if (workspace->busy) {
    rb_raise(rb_eRuntimeError, "workspace is already in use");
  }
  if (workspace->workspace != NULL) {
    gsl_integration_lebedev_free(workspace->workspace);
    workspace->workspace = NULL;
  }
  return Qnil;
}

static VALUE integration_lebedev_workspace_yield(VALUE workspace) {
  return rb_yield(workspace);
}

static VALUE integration_lebedev_workspace_open(int argc, VALUE *argv,
                                                VALUE klass) {
  VALUE workspace =
      rb_funcallv_kw(klass, rb_intern("new"), argc, argv, RB_PASS_KEYWORDS);
  return rb_ensure(integration_lebedev_workspace_yield, workspace,
                   integration_lebedev_workspace_close, workspace);
}

static integration_lebedev_workspace *
integration_lebedev_workspace_get(VALUE workspace_value) {
  integration_lebedev_workspace *workspace;

  if (!rb_obj_is_kind_of(workspace_value, integration_cLebedevWorkspace)) {
    rb_raise(rb_eTypeError,
             "workspace must be a Numo::GSL::Integration::LebedevWorkspace");
  }
  TypedData_Get_Struct(workspace_value, integration_lebedev_workspace,
                       &integration_lebedev_workspace_type, workspace);
  if (workspace->workspace == NULL) {
    rb_raise(rb_eRuntimeError, "workspace is closed");
  }
  if (workspace->busy) {
    rb_raise(rb_eRuntimeError, "workspace is already in use");
  }
  return workspace;
}

static void integration_lebedev_workspace_use_begin(
    VALUE workspace_value, size_t n, integration_lebedev_workspace_use *use) {
  integration_lebedev_workspace *workspace;

  use->workspace = NULL;
  use->owner = NULL;

  if (workspace_value == Qundef) {
    use->workspace = gsl_integration_lebedev_alloc(n);
    if (use->workspace == NULL) {
      rb_memerror();
    }
    return;
  }

  workspace = integration_lebedev_workspace_get(workspace_value);
  workspace->busy = 1;
  use->workspace = workspace->workspace;
  use->owner = workspace;
}

static VALUE integration_lebedev_workspace_use_end(VALUE data) {
  integration_lebedev_workspace_use *use =
      (integration_lebedev_workspace_use *)data;

  if (use->owner != NULL) {
    use->owner->busy = 0;
  } else if (use->workspace != NULL) {
    gsl_integration_lebedev_free(use->workspace);
  }
  use->workspace = NULL;
  use->owner = NULL;
  return Qnil;
}

static VALUE integration_call_lebedev_block(VALUE data) {
  integration_lebedev_params *params = (integration_lebedev_params *)data;
  VALUE arguments[3] = {params->x, params->y, params->z};
  VALUE value;

  value = rb_funcallv(params->block, rb_intern("call"), 3, arguments);
  return rb_Float(value);
}

static VALUE integration_run_lebedev(VALUE data) {
  integration_lebedev_params *params = (integration_lebedev_params *)data;
  double result = 0.0;
  size_t i;

  for (i = 0; i < params->workspace->n; ++i) {
    VALUE value;

    params->x = DBL2NUM(params->workspace->x[i]);
    params->y = DBL2NUM(params->workspace->y[i]);
    params->z = DBL2NUM(params->workspace->z[i]);
    value = rb_protect(integration_call_lebedev_block, (VALUE)params,
                       &params->state);

    if (params->state != 0) {
      params->exception = rb_errinfo();
      break;
    }

    result += params->workspace->weights[i] * NUM2DBL(value);
  }

  params->result = 4.0 * M_PI * result;
  return Qnil;
}

VALUE integration_lebedev(int argc, VALUE *argv, VALUE self) {
  VALUE n_value = Qundef;
  VALUE options;
  VALUE workspace_value = Qundef;
  ID keywords[] = {integration_lebedev_id_workspace};
  VALUE keyword_values[1];
  integration_lebedev_params params = {0};
  integration_lebedev_workspace_use workspace_use;
  size_t n = 0;
  int positional_count;
  int state = 0;

  (void)self;
  positional_count = rb_scan_args(argc, argv, "01:", &n_value, &options);
  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 1, keyword_values);
    workspace_value = keyword_values[0];
  }
  if (positional_count == 0 && workspace_value == Qundef) {
    rb_raise(rb_eArgError, "wrong number of arguments (given 0, expected 1)");
  }
  if (positional_count == 1 && workspace_value != Qundef) {
    rb_raise(rb_eArgError,
             "workspace cannot be specified with a Lebedev node count");
  }
  if (positional_count == 1) {
    n = integration_lebedev_node_count(n_value);
  }

  params.block = rb_block_proc();
  integration_lebedev_workspace_use_begin(workspace_value, n, &workspace_use);
  params.workspace = workspace_use.workspace;
  rb_protect(integration_run_lebedev, (VALUE)&params, &state);
  integration_lebedev_workspace_use_end((VALUE)&workspace_use);
  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.x);
  RB_GC_GUARD(params.y);
  RB_GC_GUARD(params.z);
  RB_GC_GUARD(params.exception);

  if (params.state != 0) {
    rb_exc_raise(params.exception);
  }
  if (state != 0) {
    rb_jump_tag(state);
  }

  return DBL2NUM(params.result);
}

void integration_lebedev_init(VALUE integration_module) {
  integration_lebedev_id_n = rb_intern("n");
  integration_lebedev_id_workspace = rb_intern("workspace");
  integration_lebedev_id_less_than_or_equal = rb_intern("<=");

  integration_cLebedevWorkspace =
      rb_define_class_under(integration_module, "LebedevWorkspace", rb_cObject);
  rb_define_alloc_func(integration_cLebedevWorkspace,
                       integration_lebedev_workspace_allocate);
  rb_define_method(integration_cLebedevWorkspace, "initialize",
                   integration_lebedev_workspace_initialize, -1);
  rb_define_method(integration_cLebedevWorkspace, "n",
                   integration_lebedev_workspace_n, 0);
  rb_define_method(integration_cLebedevWorkspace, "closed?",
                   integration_lebedev_workspace_closed_p, 0);
  rb_define_method(integration_cLebedevWorkspace, "close",
                   integration_lebedev_workspace_close, 0);
  rb_define_singleton_method(integration_cLebedevWorkspace, "open",
                             integration_lebedev_workspace_open, -1);
}
