# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration::LebedevWorkspace do
  describe ".new" do
    it "allocates a workspace with the requested node count", :aggregate_failures do
      workspace = described_class.new(n: 26)

      expect(workspace.n).to(eq(26))
      expect(workspace).not_to(be_closed)
    ensure
      workspace&.close
    end

    it "requires n to be an Integer" do
      expect do
        described_class.new(n: 26.0)
      end
        .to(raise_error(TypeError, "n must be an Integer"))
    end

    it "requires a positive node count" do
      expect do
        described_class.new(n: 0)
      end
        .to(raise_error(ArgumentError, "n must be positive"))
    end

    it "rejects an unsupported node count" do
      expect do
        described_class.new(n: 7)
      end
        .to(raise_error(Numo::GSL::GslError))
    end

    it "rejects reinitializing an open workspace" do
      workspace = described_class.new(n: 26)

      expect do
        workspace.send(:initialize, n: 26)
      end
        .to(raise_error(RuntimeError, "workspace is already initialized"))
    ensure
      workspace&.close
    end
  end

  describe ".open" do
    it "yields a workspace and returns the block result", :aggregate_failures do
      yielded_workspace = nil
      result = described_class.open(n: 26) do |workspace|
        yielded_workspace = workspace
        :completed
      end

      expect(result).to(eq(:completed))
      expect(yielded_workspace.n).to(eq(26))
      expect(yielded_workspace).to(be_closed)
    end

    it "closes the workspace when the block raises", :aggregate_failures do
      yielded_workspace = nil
      error_class = Class.new(StandardError)

      expect do
        described_class.open(n: 26) do |workspace|
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
      workspace = described_class.new(n: 26)
      workspace.close

      expect(workspace).to(be_closed)
    end

    it "is idempotent", :aggregate_failures do
      workspace = described_class.new(n: 26)

      expect do
        2.times { workspace.close }
      end
        .not_to(raise_error)
      expect(workspace).to(be_closed)
    end
  end
end
