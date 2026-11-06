# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration::Workspace do
  describe ".new" do
    it "allocates a workspace with the requested capacity", :aggregate_failures do
      workspace = described_class.new(limit: 100)

      expect(workspace.limit).to(eq(100))
      expect(workspace).not_to(be_closed)
    ensure
      workspace&.close
    end

    it "requires limit to be an Integer" do
      expect do
        described_class.new(limit: 100.0)
      end
        .to(raise_error(TypeError, "limit must be an Integer"))
    end

    it "requires a positive limit" do
      [0, -1].each do |limit|
        expect do
          described_class.new(limit: limit)
        end
          .to(raise_error(ArgumentError, "limit must be positive"))
      end
    end
  end

  describe ".open" do
    it "yields a workspace and returns the block result", :aggregate_failures do
      yielded_workspace = nil

      result = described_class.open(limit: 100) do |workspace|
        yielded_workspace = workspace
        :completed
      end

      expect(result).to(eq(:completed))
      expect(yielded_workspace).to(be_closed)
    end

    it "closes the workspace when the block raises" do
      yielded_workspace = nil
      error_class = Class.new(StandardError)

      expect do
        described_class.open(limit: 100) do |workspace|
          yielded_workspace = workspace
          raise error_class, "failed"
        end
      end
        .to(raise_error(error_class, "failed"))
      expect(yielded_workspace).to(be_closed)
    end
  end

  describe "#close" do
    it "closes the workspace" do
      workspace = described_class.new(limit: 100)

      workspace.close

      expect(workspace).to(be_closed)
    end

    it "is idempotent" do
      workspace = described_class.new(limit: 100)

      expect do
        2.times { workspace.close }
      end
        .not_to(raise_error)
      expect(workspace).to(be_closed)
    end
  end

  describe "use by an integration routine" do
    let(:workspace) { described_class.new(limit: 100) }

    after do
      workspace.close
    end

    it "rejects reentrant use of the same workspace" do
      expect do
        Numo::GSL::Integration.qag(
          0.0,
          1.0,
          workspace: workspace
        ) do |x|
          Numo::GSL::Integration.qag(
            0.0,
            1.0,
            workspace: workspace
          ) { |inner_x| inner_x }
          x
        end
      end
        .to(raise_error(RuntimeError, "workspace is already in use"))
    end

    it "cannot be closed while it is in use" do
      expect do
        Numo::GSL::Integration.qag(
          0.0,
          1.0,
          workspace: workspace
        ) do |x|
          workspace.close
          x
        end
      end
        .to(raise_error(RuntimeError, "workspace is already in use"))
    end

    it "can be reused after the integrand raises", :aggregate_failures do
      error_class = Class.new(StandardError)

      expect do
        Numo::GSL::Integration.qag(
          0.0,
          1.0,
          workspace: workspace
        ) { raise error_class, "failed" }
      end
        .to(raise_error(error_class, "failed"))

      result, error = Numo::GSL::Integration.qag(
        0.0,
        1.0,
        workspace: workspace
      ) { |x| x * x }

      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(error).to(be >= 0.0)
    end

    it "can be reused after GSL raises", :aggregate_failures do
      expect do
        Numo::GSL::Integration.qag(
          0.0,
          1.0,
          epsabs: 0.0,
          epsrel: 0.0,
          workspace: workspace
        ) { |x| x * x }
      end
        .to(raise_error(Numo::GSL::GslError))

      result, error = Numo::GSL::Integration.qag(
        0.0,
        1.0,
        workspace: workspace
      ) { |x| x * x }

      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(error).to(be >= 0.0)
    end
  end
end
