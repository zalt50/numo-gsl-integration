# frozen_string_literal: true
RSpec.describe Numo::GSL::Integration do
  describe ".qagp" do
    let(:integrate) do
      -> (&block) { described_class.qagp([0.0, 0.5, 1.0], &block) }
    end

    let(:workspace_expected_result) { 1.0 / 3.0 }

    let(:integrate_with_workspace) do
      lambda do |workspace, **options|
        described_class.qagp(
          [0.0, 0.25, 0.75, 1.0],
          workspace: workspace,
          **options
        ) { |x| x * x }
      end
    end

    it_behaves_like("an integration callback")
    it_behaves_like("a reusable integration workspace")
    it(
      "integrates a function with a known interior singularity",
      :aggregate_failures
    ) do
      result, error = described_class.qagp(
        [0.0, 0.5, 1.0]
      ) { |x| 1.0 / Math.sqrt((x - 0.5).abs) }
      expect(result).to(be_within(1e-10).of(2.0 * Math.sqrt(2.0)))
      expect(error).to(be >= 0.0)
    end

    it(
      "integrates a smooth function over specified subintervals",
      :aggregate_failures
    ) do
      result, error = described_class.qagp(
        [0.0, 0.25, 0.75, 1.0]
      ) { |x| x * x }
      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qagp(
          [0.0, 0.5, 1.0],
          epsabs: 0.0,
          epsrel: 0.0
        ) { |x| x * x }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class.qagp(
        [0.0, 0.5, 1.0],
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 100
      ) { |x| x * x }
      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(error).to(be >= 0.0)
    end

    it "requires points to be an Array" do
      expect do
        described_class.qagp(0.0..1.0) { |x| x * x }
      end
        .to(
          raise_error(
            TypeError,
            "points must be an Array"
          )
        )
    end

    it "requires at least two points" do
      expect do
        described_class.qagp([0.0]) { |x| x * x }
      end
        .to(
          raise_error(
            ArgumentError,
            "points must contain at least two values"
          )
        )
    end

    it "requires numeric points" do
      expect do
        described_class.qagp([0.0, "0.5", 1.0]) { |x| x * x }
      end
        .to(
          raise_error(
            TypeError,
            "points must contain only numeric values"
          )
        )
    end

    it "requires finite points" do
      [Float::NAN, Float::INFINITY, -Float::INFINITY].each do |value|
        expect do
          described_class.qagp([0.0, value, 1.0]) { |x| x * x }
        end
          .to(
            raise_error(
              ArgumentError,
              "points must contain only finite values"
            )
          )
      end
    end

    it "requires points to be strictly increasing" do
      [[0.0, 0.5, 0.5, 1.0], [0.0, 0.75, 0.25, 1.0]].each do |points|
        expect do
          described_class.qagp(points) { |x| x * x }
        end
          .to(
            raise_error(
              ArgumentError,
              "points must be strictly increasing"
            )
          )
      end
    end
  end
end
