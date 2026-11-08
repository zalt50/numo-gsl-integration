# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".romberg" do

    let(:integrate) do
      -> (&block) { described_class.romberg(0.0, 1.0, &block) }
    end

    it_behaves_like("an integration callback")

    it "integrates a smooth function over a finite interval", :aggregate_failures do
      result, evaluations = described_class.romberg(0.0, Math::PI) do |x|
        Math.sin(x)
      end

      expect(result).to(be_within(1e-10).of(2.0))
      expect(evaluations).to(be_positive)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.romberg(
          0.0,
          1.0,
          epsabs: -1.0
        ) { |x| x * x }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, evaluations = described_class.romberg(
        0.0,
        1.0,
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 20
      ) { |x| x * x }

      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(evaluations).to(be_positive)
    end

    it "requires a limit between one and thirty" do
      [-1, 0, 31].each do |limit|
        expect do
          described_class.romberg(0.0, 1.0, limit: limit) { |x| x * x }
        end
          .to(raise_error(ArgumentError, "limit must be between 1 and 30"))
      end
    end

    it "accepts the minimum limit", :aggregate_failures do
      result, evaluations = described_class.romberg(
        0.0,
        Math::PI,
        limit: 1
      ) { |x| Math.sin(x) }

      expect(result).to(be_a(Float))
      expect(evaluations).to(eq(2))
    end

    it "accepts the maximum limit", :aggregate_failures do
      result, evaluations = described_class.romberg(
        0.0,
        Math::PI,
        limit: 30
      ) { |x| Math.sin(x) }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(evaluations).to(be_positive)
    end

    it "rejects an explicit nil limit" do
      expect do
        described_class.romberg(0.0, 1.0, limit: nil) { |x| x * x }
      end
        .to(raise_error(TypeError))
    end

    context("with a reusable workspace") do
      let(:workspace) { described_class::RombergWorkspace.new(limit: 20) }
      let(:integrate_with_workspace) do
        lambda do |**options|
          described_class.romberg(
            0.0,
            Math::PI,
            workspace: workspace,
            **options
          ) { |x| Math.sin(x) }
        end
      end

      after do
        workspace.close unless workspace.closed?
      end

      it "reuses the same workspace across calls", :aggregate_failures do
        first_result, first_evaluations = integrate_with_workspace.call
        second_result, second_evaluations = integrate_with_workspace.call

        expect(first_result).to(be_within(1e-10).of(2.0))
        expect(second_result).to(be_within(1e-10).of(2.0))
        expect(first_evaluations).to(be_positive)
        expect(second_evaluations).to(be_positive)
        expect(workspace).not_to(be_closed)
      end

      it "uses the workspace capacity when limit is omitted" do
        result, = integrate_with_workspace.call

        expect(result).to(be_within(1e-10).of(2.0))
      end

      it "accepts a limit matching the workspace capacity" do
        result, = integrate_with_workspace.call(limit: 20)

        expect(result).to(be_within(1e-10).of(2.0))
      end

      it "rejects a limit different from the workspace capacity" do
        [10, 30].each do |limit|
          expect do
            integrate_with_workspace.call(limit: limit)
          end
            .to(
              raise_error(
                ArgumentError,
                "limit must match workspace capacity"
              )
            )
        end
      end

      it "rejects a closed workspace" do
        workspace.close

        expect do
          integrate_with_workspace.call
        end
          .to(raise_error(RuntimeError, "workspace is closed"))
      end

      it "requires workspace to be a RombergWorkspace" do
        [nil, Object.new].each do |invalid_workspace|
          expect do
            described_class.romberg(
              0.0,
              Math::PI,
              workspace: invalid_workspace
            ) { |x| Math.sin(x) }
          end
            .to(
              raise_error(
                TypeError,
                "workspace must be a Numo::GSL::Integration::RombergWorkspace"
              )
            )
        end
      end
    end
  end
end
