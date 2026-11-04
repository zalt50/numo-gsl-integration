#include "integration_internal.h"

VALUE integration_free_workspace(VALUE data) {
  gsl_integration_workspace *workspace = (gsl_integration_workspace *)data;

  gsl_integration_workspace_free(workspace);

  return Qnil;
}

VALUE integration_free_cquad_workspace(VALUE data) {
  gsl_integration_cquad_workspace *workspace =
      (gsl_integration_cquad_workspace *)data;

  gsl_integration_cquad_workspace_free(workspace);

  return Qnil;
}

VALUE integration_free_romberg_workspace(VALUE data) {
  gsl_integration_romberg_workspace *workspace =
      (gsl_integration_romberg_workspace *)data;

  gsl_integration_romberg_free(workspace);

  return Qnil;
}

VALUE integration_free_glfixed_table(VALUE data) {
  gsl_integration_glfixed_table *table = (gsl_integration_glfixed_table *)data;

  gsl_integration_glfixed_table_free(table);

  return Qnil;
}
