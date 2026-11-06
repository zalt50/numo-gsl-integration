# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".qawc" do

    let(:integrate) do
      -> (&block) { described_class.qawc(-1.0, 1.0, 0.0, &block) }
    end

    let(:integrate_with_workspace) do
      lambda do |workspace, **options|
        described_class.qawc(
          0.0,
          2.0,
          1.0,
          workspace: workspace,
          **options
        ) { |x| x }
      end
    end

    let(:workspace_expected_result) { 2.0 }

    it_behaves_like("an integration callback")
    it_behaves_like("a reusable integration workspace")

    it "computes a Cauchy principal value", :aggregate_failures do
      result, error = described_class.qawc(0.0, 2.0, 1.0) { |x| x }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qawc(
          -1.0,
          1.0,
          0.0,
          epsabs: 0.0,
          epsrel: 0.0
        ) { 1.0 }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class.qawc(
        -1.0,
        1.0,
        0.0,
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 100
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(0.0))
      expect(error).to(be >= 0.0)
    end
  end
end
