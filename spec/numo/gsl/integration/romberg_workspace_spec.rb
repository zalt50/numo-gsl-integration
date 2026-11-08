# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration::RombergWorkspace do
  describe ".new" do
    it "allocates a workspace with the requested capacity", :aggregate_failures do
      workspace = described_class.new(limit: 20)

      expect(workspace.limit).to(eq(20))
      expect(workspace).not_to(be_closed)
    ensure
      workspace&.close
    end

    it "requires limit to be an Integer" do
      expect do
        described_class.new(limit: 20.0)
      end
        .to(raise_error(TypeError, "limit must be an Integer"))
    end

    it "requires a limit between one and thirty" do
      [-1, 0, 31].each do |limit|
        expect do
          described_class.new(limit: limit)
        end
          .to(raise_error(ArgumentError, "limit must be between 1 and 30"))
      end
    end

    it "accepts the boundary capacities", :aggregate_failures do
      minimum = described_class.new(limit: 1)
      maximum = described_class.new(limit: 30)

      expect(minimum.limit).to(eq(1))
      expect(maximum.limit).to(eq(30))
      expect(minimum).not_to(be_closed)
      expect(maximum).not_to(be_closed)
    ensure
      minimum&.close
      maximum&.close
    end

    it "rejects reinitializing an open workspace" do
      workspace = described_class.new(limit: 20)

      expect do
        workspace.send(:initialize, limit: 20)
      end
        .to(raise_error(RuntimeError, "workspace is already initialized"))
    ensure
      workspace&.close
    end
  end

  describe ".open" do
    it "yields a workspace and returns the block result", :aggregate_failures do
      yielded_workspace = nil
      result = described_class.open(limit: 30) do |workspace|
        yielded_workspace = workspace
        :completed
      end

      expect(result).to(eq(:completed))
      expect(yielded_workspace.limit).to(eq(30))
      expect(yielded_workspace).to(be_closed)
    end

    it "closes the workspace when the block raises", :aggregate_failures do
      yielded_workspace = nil
      error_class = Class.new(StandardError)

      expect do
        described_class.open(limit: 20) do |workspace|
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
      workspace = described_class.new(limit: 20)

      workspace.close

      expect(workspace).to(be_closed)
    end

    it "is idempotent", :aggregate_failures do
      workspace = described_class.new(limit: 20)

      expect do
        2.times { workspace.close }
      end
        .not_to(raise_error)
      expect(workspace).to(be_closed)
    end
  end

  describe "use by romberg" do
    let(:workspace) { described_class.new(limit: 20) }
    let(:integrate_sine) do
      lambda do
        Numo::GSL::Integration.romberg(
          0.0,
          Math::PI,
          workspace: workspace
        ) { |x| Math.sin(x) }
      end
    end

    after do
      workspace.close
    end

    it "rejects reentrant use of the same workspace" do
      expect do
        Numo::GSL::Integration.romberg(
          0.0,
          1.0,
          workspace: workspace
        ) do |x|
          Numo::GSL::Integration.romberg(
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
        Numo::GSL::Integration.romberg(
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
        Numo::GSL::Integration.romberg(
          0.0,
          1.0,
          workspace: workspace
        ) { raise error_class, "failed" }
      end
        .to(raise_error(error_class, "failed"))

      result, evaluations = integrate_sine.call

      expect(result).to(be_within(1e-10).of(2.0))
      expect(evaluations).to(be_positive)
    end

    it "can be reused after GSL raises", :aggregate_failures do
      expect do
        Numo::GSL::Integration.romberg(
          0.0,
          1.0,
          epsabs: -1.0,
          workspace: workspace
        ) { |x| x * x }
      end
        .to(raise_error(Numo::GSL::GslError))

      result, evaluations = integrate_sine.call

      expect(result).to(be_within(1e-10).of(2.0))
      expect(evaluations).to(be_positive)
    end

    it "uses the minimum native workspace capacity", :aggregate_failures do
      minimum_workspace = described_class.new(limit: 1)
      result, evaluations = Numo::GSL::Integration.romberg(
        0.0,
        Math::PI,
        workspace: minimum_workspace
      ) { |x| Math.sin(x) }

      expect(result).to(be_a(Float))
      expect(evaluations).to(eq(2))
    ensure
      minimum_workspace&.close
    end

    it "uses the maximum native workspace capacity", :aggregate_failures do
      maximum_workspace = described_class.new(limit: 30)
      result, evaluations = Numo::GSL::Integration.romberg(
        0.0,
        Math::PI,
        workspace: maximum_workspace
      ) { |x| Math.sin(x) }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(evaluations).to(be_positive)
    ensure
      maximum_workspace&.close
    end
  end
end
