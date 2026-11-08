# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".lebedev" do
    let(:integrate) do
      lambda do |&block|
        described_class.lebedev(26, &block)
      end
    end

    it_behaves_like("an integration callback")

    it "integrates a constant over the unit sphere" do
      result = described_class.lebedev(6) do |_x, _y, _z|
        1.0
      end

      expect(result).to(be_within(1e-14).of(4.0 * Math::PI))
    end

    it "integrates an odd function to zero" do
      result = described_class.lebedev(14) do |x, _y, _z|
        x
      end

      expect(result).to(be_within(1e-14).of(0.0))
    end

    it "integrates a quadratic coordinate" do
      result = described_class.lebedev(14) do |x, _y, _z|
        x * x
      end

      expect(result).to(be_within(1e-14).of(4.0 * Math::PI / 3.0))
    end

    it "passes points on the unit sphere to the block" do
      result = described_class.lebedev(26) do |x, y, z|
        (x * x) + (y * y) + (z * z)
      end

      expect(result).to(be_within(1e-14).of(4.0 * Math::PI))
    end

    it "requires an integer node count" do
      expect do
        described_class.lebedev("6") { |_x, _y, _z| 1.0 }
      end
        .to(raise_error(TypeError, "n must be an Integer"))
    end

    it "requires a positive node count" do
      expect do
        described_class.lebedev(0) { |_x, _y, _z| 1.0 }
      end
        .to(raise_error(ArgumentError, "n must be positive"))
    end

    it "rejects an unsupported node count" do
      expect do
        described_class.lebedev(7) { |_x, _y, _z| 1.0 }
      end
        .to(raise_error(Numo::GSL::GslError))
    end

    context("with a reusable workspace") do
      let(:workspace) do
        Numo::GSL::Integration::LebedevWorkspace.new(n: 26)
      end

      let(:integrate_coordinate) do
        lambda do |coordinate|
          described_class.lebedev(workspace: workspace) do |x, y, _z|
            value = coordinate == :x ? x : y
            value * value
          end
        end
      end

      after do
        workspace.close
      end

      it "integrates with the nodes stored in the workspace" do
        result = described_class.lebedev(workspace: workspace) do |x, y, z|
          (x * x) + (y * y) + (z * z)
        end

        expect(result).to(be_within(1e-14).of(4.0 * Math::PI))
      end

      it "reuses the same workspace for different integrands", :aggregate_failures do
        x_result = integrate_coordinate.call(:x)
        y_result = integrate_coordinate.call(:y)

        expect(x_result).to(be_within(1e-14).of(4.0 * Math::PI / 3.0))
        expect(y_result).to(be_within(1e-14).of(4.0 * Math::PI / 3.0))
        expect(workspace).not_to(be_closed)
      end

      it "rejects a node count with a workspace" do
        expect do
          described_class.lebedev(26, workspace: workspace) do |_x, _y, _z|
            1.0
          end
        end
          .to(
            raise_error(
              ArgumentError,
              "workspace cannot be specified with a Lebedev node count"
            )
          )
      end

      it "rejects reentrant use of the same workspace" do
        expect do
          described_class.lebedev(workspace: workspace) do |x, _y, _z|
            described_class.lebedev(workspace: workspace) { 1.0 }
            x
          end
        end
          .to(raise_error(RuntimeError, "workspace is already in use"))
      end

      it "cannot be closed while it is in use" do
        expect do
          described_class.lebedev(workspace: workspace) do |x, _y, _z|
            workspace.close
            x
          end
        end
          .to(raise_error(RuntimeError, "workspace is already in use"))
      end

      it "can be reused after the integrand raises", :aggregate_failures do
        error_class = Class.new(StandardError)

        expect do
          described_class.lebedev(workspace: workspace) do
            raise error_class, "failed"
          end
        end
          .to(raise_error(error_class, "failed"))

        expect(integrate_coordinate.call(:x))
          .to(be_within(1e-14).of(4.0 * Math::PI / 3.0))
      end

      it "rejects a closed workspace" do
        workspace.close

        expect do
          described_class.lebedev(workspace: workspace) { 1.0 }
        end
          .to(raise_error(RuntimeError, "workspace is closed"))
      end

      it "requires a LebedevWorkspace" do
        expect do
          described_class.lebedev(workspace: Object.new) { 1.0 }
        end
          .to(
            raise_error(
              TypeError,
              "workspace must be a Numo::GSL::Integration::LebedevWorkspace"
            )
          )
      end
    end
  end
end
