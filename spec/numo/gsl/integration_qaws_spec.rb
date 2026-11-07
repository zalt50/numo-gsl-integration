# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".qaws" do

    let(:integrate) do
      lambda do |&block|
        described_class.qaws(0.0, 1.0, 0.0, 0.0, 0, 0, &block)
      end
    end

    let(:workspace_expected_result) { 1.0 }

    let(:integrate_with_workspace) do
      lambda do |workspace, **options|
        described_class.qaws(
          0.0,
          1.0,
          0.0,
          0.0,
          0,
          0,
          workspace: workspace,
          **options
        ) { 1.0 }
      end
    end

    it_behaves_like("an integration callback")
    it_behaves_like("a reusable integration workspace")

    context("with a reusable QAWS table") do
      let(:qaws_table) do
        described_class::QAWSTable.new(
          alpha: -0.5,
          beta: -0.5,
          mu: 0,
          nu: 0
        )
      end

      let(:integrate_with_table) do
        lambda do
          described_class.qaws(
            0.0,
            1.0,
            -0.5,
            -0.5,
            0,
            0,
            table: qaws_table
          ) { 1.0 }
        end
      end

      after do
        qaws_table.close unless qaws_table.closed?
      end

      it "reuses the same QAWS table across calls", :aggregate_failures do
        first_result, first_error = integrate_with_table.call
        second_result, second_error = integrate_with_table.call

        expect(first_result).to(be_within(1e-10).of(Math::PI))
        expect(second_result).to(be_within(1e-10).of(Math::PI))
        expect(first_error).to(be >= 0.0)
        expect(second_error).to(be >= 0.0)
        expect(qaws_table).not_to(be_closed)
      end

      it "accepts a reusable workspace and QAWS table together" do
        workspace = described_class::Workspace.new(limit: 50)
        result, = described_class.qaws(
          0.0,
          1.0,
          -0.5,
          -0.5,
          0,
          0,
          workspace: workspace,
          table: qaws_table
        ) { 1.0 }

        expect(result).to(be_within(1e-10).of(Math::PI))
      ensure
        workspace&.close
      end

      it "rejects a closed QAWS table" do
        qaws_table.close

        expect do
          integrate_with_table.call
        end
          .to(raise_error(RuntimeError, "table is closed"))
      end

      it "requires alpha to match the QAWS table" do
        expect do
          described_class.qaws(
            0.0,
            1.0,
            -0.25,
            -0.5,
            0,
            0,
            table: qaws_table
          ) { 1.0 }
        end
          .to(raise_error(ArgumentError, "alpha does not match QAWS table"))
      end

      it "requires beta to match the QAWS table" do
        expect do
          described_class.qaws(
            0.0,
            1.0,
            -0.5,
            -0.25,
            0,
            0,
            table: qaws_table
          ) { 1.0 }
        end
          .to(raise_error(ArgumentError, "beta does not match QAWS table"))
      end

      it "requires mu to match the QAWS table" do
        expect do
          described_class.qaws(
            0.0,
            1.0,
            -0.5,
            -0.5,
            1,
            0,
            table: qaws_table
          ) { 1.0 }
        end
          .to(raise_error(ArgumentError, "mu does not match QAWS table"))
      end

      it "requires nu to match the QAWS table" do
        expect do
          described_class.qaws(
            0.0,
            1.0,
            -0.5,
            -0.5,
            0,
            1,
            table: qaws_table
          ) { 1.0 }
        end
          .to(raise_error(ArgumentError, "nu does not match QAWS table"))
      end
    end

    context("with an invalid table") do
      let(:integrate_with_invalid_table) do
        lambda do
          described_class.qaws(
            0.0,
            1.0,
            -0.5,
            -0.5,
            0,
            0,
            table: Object.new
          ) { 1.0 }
        end
      end

      it "requires table to be an Integration::QAWSTable" do
        expect do
          integrate_with_invalid_table.call
        end
          .to(
            raise_error(
              TypeError,
              "table must be a Numo::GSL::Integration::QAWSTable"
            )
          )
      end
    end

    it "integrates a function with algebraic endpoint singularities", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        -0.5,
        -0.5,
        0,
        0
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(Math::PI))
      expect(error).to(be >= 0.0)
    end

    it "applies a logarithmic weight at the lower endpoint", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        0.0,
        0.0,
        1,
        0
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(-1.0))
      expect(error).to(be >= 0.0)
    end

    it "applies a logarithmic weight at the upper endpoint", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        0.0,
        0.0,
        0,
        1
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(-1.0))
      expect(error).to(be >= 0.0)
    end

    it "applies logarithmic weights at both endpoints", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        0.0,
        0.0,
        1,
        1
      ) { 1.0 }
      expected = 2.0 - ((Math::PI ** 2) / 6.0)

      expect(result).to(be_within(1e-10).of(expected))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qaws(
          0.0,
          1.0,
          0.0,
          0.0,
          0,
          0,
          epsabs: 0.0,
          epsrel: 0.0
        ) { 1.0 }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        -0.5,
        0.0,
        0,
        0,
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 100
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(error).to(be >= 0.0)
    end
  end
end
