# frozen_string_literal: true

RSpec.shared_examples("a reusable integration workspace") do
  let(:workspace) do
    Numo::GSL::Integration::Workspace.new(limit: 50)
  end

  after do
    workspace.close unless workspace.closed?
  end

  it "reuses the same workspace across calls", :aggregate_failures do
    first_result, first_error = integrate_with_workspace.call(workspace)
    second_result, second_error = integrate_with_workspace.call(workspace)

    expect(first_result).to(be_within(1e-10).of(workspace_expected_result))
    expect(second_result).to(be_within(1e-10).of(workspace_expected_result))
    expect(first_error).to(be >= 0.0)
    expect(second_error).to(be >= 0.0)
    expect(workspace).not_to(be_closed)
  end

  it "uses the workspace when limit is omitted" do
    result, = integrate_with_workspace.call(workspace)

    expect(result).to(be_within(1e-10).of(workspace_expected_result))
  end

  it "accepts a limit within the workspace capacity" do
    result, = integrate_with_workspace.call(workspace, limit: 25)

    expect(result).to(be_within(1e-10).of(workspace_expected_result))
  end

  it "rejects a limit larger than the workspace capacity" do
    expect do
      integrate_with_workspace.call(workspace, limit: 51)
    end
      .to(
        raise_error(
          ArgumentError,
          "limit exceeds workspace capacity"
        )
      )
  end

  it "rejects a closed workspace" do
    workspace.close

    expect do
      integrate_with_workspace.call(workspace)
    end
      .to(raise_error(RuntimeError, "workspace is closed"))
  end

  it "requires workspace to be an Integration::Workspace" do
    expect do
      integrate_with_workspace.call(Object.new)
    end
      .to(
        raise_error(
          TypeError,
          "workspace must be a Numo::GSL::Integration::Workspace"
        )
      )
  end
end
