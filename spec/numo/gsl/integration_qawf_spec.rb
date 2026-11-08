# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".qawf" do

    let(:integrate) do
      lambda do |&block|
        described_class.qawf(
          0.0,
          1.0,
          described_class::COSINE,
          &block
        )
      end
    end

    it_behaves_like("an integration callback")

    it "integrates over a semi-infinite interval with a cosine weight", :aggregate_failures do
      result, error = described_class.qawf(
        0.0,
        1.0,
        described_class::COSINE
      ) { |x| Math.exp(-x) }

      expect(result).to(be_within(1e-8).of(0.5))
      expect(error).to(be >= 0.0)
    end

    it "integrates over a semi-infinite interval with a sine weight", :aggregate_failures do
      result, error = described_class.qawf(
        0.0,
        1.0,
        described_class::SINE
      ) { |x| Math.exp(-x) }

      expect(result).to(be_within(1e-8).of(0.5))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qawf(
          0.0,
          1.0,
          described_class::COSINE,
          epsabs: 0.0
        ) { |x| Math.exp(-x) }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class
        .qawf(
          0.0,
          1.0,
          described_class::SINE,
          epsabs: 1e-12,
          limit: 100
        ) { |x| Math.exp(-x) }

      expect(result).to(be_within(1e-8).of(0.5))
      expect(error).to(be >= 0.0)
    end

    it "requires sine to be an Integer" do
      expect do
        described_class.qawf(0.0, 1.0, 1.0) { |x| Math.exp(-x) }
      end
        .to(raise_error(TypeError, "sine must be an Integer"))
    end

    it "requires sine to be SINE or COSINE" do
      expect do
        described_class.qawf(0.0, 1.0, -1) { |x| Math.exp(-x) }
      end
        .to(raise_error(ArgumentError, "sine must be SINE or COSINE"))
    end

    context("with reusable workspaces") do
      let(:workspace) { described_class::Workspace.new(limit: 50) }
      let(:cycle_workspace) { described_class::Workspace.new(limit: 50) }
      let(:workspace_options) do
        {workspace: workspace, cycle_workspace: cycle_workspace}
      end

      let(:integrate_with_workspaces) do
        lambda do |options = workspace_options, &block|
          integrand = block || -> (x) { Math.exp(-x) }
          described_class.qawf(
            0.0,
            1.0,
            described_class::COSINE,
            **options,
            &integrand
          )
        end
      end

      after do
        workspace.close unless workspace.closed?
        cycle_workspace.close unless cycle_workspace.closed?
      end

      it "reuses both workspaces across calls", :aggregate_failures do
        first_result, first_error = integrate_with_workspaces.call
        second_result, second_error = integrate_with_workspaces.call

        expect(first_result).to(be_within(1e-8).of(0.5))
        expect(second_result).to(be_within(1e-8).of(0.5))
        expect(first_error).to(be >= 0.0)
        expect(second_error).to(be >= 0.0)
        expect(workspace).not_to(be_closed)
        expect(cycle_workspace).not_to(be_closed)
      end

      it "reuses only the outer workspace" do
        result, = described_class.qawf(
          0.0,
          1.0,
          described_class::COSINE,
          workspace: workspace
        ) { |x| Math.exp(-x) }

        expect(result).to(be_within(1e-8).of(0.5))
      end

      it "reuses only the cycle workspace" do
        result, = described_class.qawf(
          0.0,
          1.0,
          described_class::COSINE,
          cycle_workspace: cycle_workspace
        ) { |x| Math.exp(-x) }

        expect(result).to(be_within(1e-8).of(0.5))
      end

      it "uses the smaller workspace capacity when limit is omitted" do
        smaller = described_class::Workspace.new(limit: 25)
        options = workspace_options.merge(cycle_workspace: smaller)
        result, = integrate_with_workspaces.call(options)

        expect(result).to(be_within(1e-8).of(0.5))
      ensure
        smaller&.close
      end

      it "accepts a limit within both workspace capacities" do
        options = workspace_options.merge(limit: 25)
        result, = integrate_with_workspaces.call(options)

        expect(result).to(be_within(1e-8).of(0.5))
      end

      it "rejects a limit larger than the outer workspace capacity" do
        smaller = described_class::Workspace.new(limit: 25)
        options = workspace_options.merge(workspace: smaller, limit: 26)

        expect do
          integrate_with_workspaces.call(options)
        end
          .to(raise_error(ArgumentError, "limit exceeds workspace capacity"))
      ensure
        smaller&.close
      end

      it "rejects a limit larger than the cycle workspace capacity" do
        smaller = described_class::Workspace.new(limit: 25)
        options = workspace_options.merge(cycle_workspace: smaller, limit: 26)

        expect do
          integrate_with_workspaces.call(options)
        end
          .to(
            raise_error(
              ArgumentError,
              "limit exceeds cycle_workspace capacity"
            )
          )
      ensure
        smaller&.close
      end

      it "requires different objects for the two workspace roles" do
        expect do
          integrate_with_workspaces.call(
            {workspace: workspace, cycle_workspace: workspace}
          )
        end
          .to(
            raise_error(
              ArgumentError,
              "workspace and cycle_workspace must be different objects"
            )
          )
      end

      it "rejects a closed outer workspace" do
        workspace.close

        expect do
          integrate_with_workspaces.call
        end
          .to(raise_error(RuntimeError, "workspace is closed"))
      end

      it "rejects a closed cycle workspace" do
        cycle_workspace.close

        expect do
          integrate_with_workspaces.call
        end
          .to(raise_error(RuntimeError, "cycle_workspace is closed"))
      end

      it "requires workspace to be an Integration::Workspace" do
        options = workspace_options.merge(workspace: Object.new)

        expect do
          integrate_with_workspaces.call(options)
        end
          .to(
            raise_error(
              TypeError,
              "workspace must be a Numo::GSL::Integration::Workspace"
            )
          )
      end

      it "requires cycle_workspace to be an Integration::Workspace" do
        options = workspace_options.merge(cycle_workspace: Object.new)

        expect do
          integrate_with_workspaces.call(options)
        end
          .to(
            raise_error(
              TypeError,
              "cycle_workspace must be a Numo::GSL::Integration::Workspace"
            )
          )
      end

      it "rejects reentrant use of the outer workspace" do
        expect do
          integrate_with_workspaces.call do
            described_class.qag(0.0, 1.0, workspace: workspace) { |x| x }
          end
        end
          .to(raise_error(RuntimeError, "workspace is already in use"))
      end

      it "rejects reentrant use of the cycle workspace" do
        expect do
          integrate_with_workspaces.call do
            described_class.qag(
              0.0,
              1.0,
              workspace: cycle_workspace
            ) { |x| x }
          end
        end
          .to(raise_error(RuntimeError, "workspace is already in use"))
      end

      it "prevents the outer workspace from closing while in use" do
        expect do
          integrate_with_workspaces.call { workspace.close }
        end
          .to(raise_error(RuntimeError, "workspace is already in use"))
      end

      it "prevents the cycle workspace from closing while in use" do
        expect do
          integrate_with_workspaces.call { cycle_workspace.close }
        end
          .to(raise_error(RuntimeError, "workspace is already in use"))
      end

      it "releases both workspaces after the integrand raises", :aggregate_failures do
        error_class = Class.new(StandardError)

        expect do
          integrate_with_workspaces.call { raise error_class, "failed" }
        end
          .to(raise_error(error_class, "failed"))

        result, error = integrate_with_workspaces.call
        expect(result).to(be_within(1e-8).of(0.5))
        expect(error).to(be >= 0.0)
      end

      it "releases both workspaces after GSL raises", :aggregate_failures do
        options = workspace_options.merge(epsabs: 0.0)

        expect do
          integrate_with_workspaces.call(options)
        end
          .to(raise_error(Numo::GSL::GslError))

        result, error = integrate_with_workspaces.call
        expect(result).to(be_within(1e-8).of(0.5))
        expect(error).to(be >= 0.0)
      end
    end
  end
end
