# frozen_string_literal: true
RSpec.describe Numo::GSL::Integration do
  describe ".qagi" do

    let(:integrate) do
      -> (&block) { described_class.qagi(&block) }
    end

    let(:workspace_expected_result) { Math::PI }
    let(:integrate_with_workspace) do
      lambda do |workspace, **options|
        described_class.qagi(
          workspace: workspace,
          **options
        ) { |x| 1.0 / (1.0 + (x * x)) }
      end
    end

    it_behaves_like("an integration callback")

    it_behaves_like("a reusable integration workspace")

    it "integrates a function over an infinite interval", :aggregate_failures do
      result, error = described_class.qagi do |x|
        Math.exp(-(x * x))
      end

      expect(result).to(be_within(1e-10).of(Math.sqrt(Math::PI)))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qagi(
          epsabs: 0.0,
          epsrel: 0.0
        ) { |x| x * x }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class.qagi(
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 100
      ) { |x| 1.0 / (1.0 + (x * x)) }
      expect(result).to(be_within(1e-10).of(Math::PI))
      expect(error).to(be >= 0.0)
    end
  end
end
