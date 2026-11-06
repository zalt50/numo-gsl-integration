# frozen_string_literal: true
RSpec.describe Numo::GSL::Integration do
  describe ".qagiu" do

    let(:integrate) do
      -> (&block) { described_class.qagiu(0.0, &block) }
    end

    it_behaves_like("an integration callback")

    let(:integrate_with_workspace) do
      lambda do |workspace, **options|
        described_class.qagiu(
          0.0,
          workspace: workspace,
          **options
        ) { |x| Math.exp(-x) }
      end
    end

    let(:workspace_expected_result) { 1.0 }

    it_behaves_like("a reusable integration workspace")

    it "integrates a function over an upper semi-infinite interval", :aggregate_failures do
      result, error = described_class.qagiu(0.0) { |x| Math.exp(-x) }
      expect(result).to(be_within(1e-10).of(1.0))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qagiu(
          0.0,
          epsabs: 0.0,
          epsrel: 0.0
        ) { |x| Math.exp(-x) }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class.qagiu(
        1.0,
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 100
      ) { |x| Math.exp(-x) }
      expect(result).to(be_within(1e-10).of(Math.exp(-1.0)))
      expect(error).to(be >= 0.0)
    end
  end
end
