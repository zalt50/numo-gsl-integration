# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration::FixedWorkspace do
  describe ".new" do
    it "allocates a workspace with the requested parameters", :aggregate_failures do
      workspace = described_class.new(
        :jacobi,
        order: 10,
        a: -1.0,
        b: 1.0,
        alpha: 0.5,
        beta: 1.5
      )

      expect(workspace.type).to(eq(:jacobi))
      expect(workspace.order).to(eq(10))
      expect(workspace.a).to(eq(-1.0))
      expect(workspace.b).to(eq(1.0))
      expect(workspace.alpha).to(eq(0.5))
      expect(workspace.beta).to(eq(1.5))
      expect(workspace).not_to(be_closed)
    ensure
      workspace&.close
    end

    it "returns nil for parameters unused by the quadrature type", :aggregate_failures do
      workspace = described_class.new(
        :legendre,
        order: 10,
        a: 0.0,
        b: 1.0
      )

      expect(workspace.alpha).to(be_nil)
      expect(workspace.beta).to(be_nil)
    ensure
      workspace&.close
    end

    it "requires the quadrature type to be a Symbol" do
      expect do
        described_class.new(
          "legendre",
          order: 10,
          a: 0.0,
          b: 1.0
        )
      end
        .to(raise_error(TypeError, "type must be a Symbol"))
    end

    it "rejects an unsupported quadrature type" do
      expect do
        described_class.new(
          :unknown,
          order: 10,
          a: 0.0,
          b: 1.0
        )
      end
        .to(
          raise_error(
            ArgumentError,
            "unsupported fixed quadrature type: unknown"
          )
        )
    end

    it "requires order to be an Integer" do
      expect do
        described_class.new(
          :legendre,
          order: 10.0,
          a: 0.0,
          b: 1.0
        )
      end
        .to(raise_error(TypeError, "order must be an Integer"))
    end

    it "requires a positive order" do
      [0, -1].each do |order|
        expect do
          described_class.new(
            :legendre,
            order: order,
            a: 0.0,
            b: 1.0
          )
        end
          .to(raise_error(ArgumentError, "order must be positive"))
      end
    end

    it "requires alpha for quadrature types that use it" do
      expect do
        described_class.new(
          :gegenbauer,
          order: 10,
          a: -1.0,
          b: 1.0
        )
      end
        .to(raise_error(ArgumentError, "alpha is required for gegenbauer"))
    end

    it "requires beta for quadrature types that use it" do
      expect do
        described_class.new(
          :jacobi,
          order: 10,
          a: -1.0,
          b: 1.0,
          alpha: 0.5
        )
      end
        .to(raise_error(ArgumentError, "beta is required for jacobi"))
    end

    it "rejects reinitializing an open workspace" do
      workspace = described_class.new(
        :legendre,
        order: 10,
        a: 0.0,
        b: 1.0
      )

      expect do
        workspace.send(
          :initialize,
          :legendre,
          order: 10,
          a: 0.0,
          b: 1.0
        )
      end
        .to(raise_error(RuntimeError, "workspace is already initialized"))
    ensure
      workspace&.close
    end
  end

  describe ".open" do
    it "yields a workspace and returns the block result", :aggregate_failures do
      yielded_workspace = nil
      result = described_class.open(
        :legendre,
        order: 8,
        a: 0.0,
        b: 1.0
      ) do |workspace|
        yielded_workspace = workspace
        :completed
      end

      expect(result).to(eq(:completed))
      expect(yielded_workspace.order).to(eq(8))
      expect(yielded_workspace).to(be_closed)
    end

    it "closes the workspace when the block raises", :aggregate_failures do
      yielded_workspace = nil
      error_class = Class.new(StandardError)

      expect do
        described_class.open(
          :legendre,
          order: 10,
          a: 0.0,
          b: 1.0
        ) do |workspace|
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
      workspace = described_class.new(
        :legendre,
        order: 10,
        a: 0.0,
        b: 1.0
      )

      workspace.close

      expect(workspace).to(be_closed)
    end

    it "is idempotent", :aggregate_failures do
      workspace = described_class.new(
        :legendre,
        order: 10,
        a: 0.0,
        b: 1.0
      )

      expect do
        2.times { workspace.close }
      end
        .not_to(raise_error)
      expect(workspace).to(be_closed)
    end
  end
end
