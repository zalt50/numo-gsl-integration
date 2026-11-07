# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".qawo" do

    let(:integrate) do
      lambda do |&block|
        described_class.qawo(
          0.0,
          1.0,
          1.0,
          described_class::COSINE,
          &block
        )
      end
    end

    let(:workspace_expected_result) { 2.0 }

    let(:integrate_with_workspace) do
      lambda do |workspace, **options|
        described_class.qawo(
          0.0,
          Math::PI,
          1.0,
          described_class::SINE,
          workspace: workspace,
          **options
        ) { 1.0 }
      end
    end

    it_behaves_like("an integration callback")
    it_behaves_like("a reusable integration workspace")

    context("with a reusable QAWO table") do
      let(:qawo_table) do
        @qawo_table = described_class::QAWOTable.new(
          omega: 1.0,
          length: Math::PI,
          sine: described_class::SINE,
          levels: 50
        )
      end

      after do
        @qawo_table&.close unless @qawo_table&.closed?
      end

      it "reuses the same QAWO table across calls", :aggregate_failures do
        first_result, first_error = described_class.qawo(
          0.0,
          Math::PI,
          1.0,
          described_class::SINE,
          table: qawo_table
        ) { 1.0 }
        second_result, second_error = described_class
          .qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::SINE,
            table: qawo_table
          ) { 1.0 }

        expect(first_result).to(be_within(1e-10).of(2.0))
        expect(second_result).to(be_within(1e-10).of(2.0))
        expect(first_error).to(be >= 0.0)
        expect(second_error).to(be >= 0.0)
        expect(qawo_table).not_to(be_closed)
      end

      it "accepts a reusable workspace and QAWO table together" do
        workspace = described_class::Workspace.new(limit: 50)
        result, = described_class
          .qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::SINE,
            workspace: workspace,
            table: qawo_table
          ) { 1.0 }

        expect(result).to(be_within(1e-10).of(2.0))
      ensure
        workspace&.close
      end

      it "rejects a closed QAWO table" do
        qawo_table.close

        expect do
          described_class.qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::SINE,
            table: qawo_table
          ) { 1.0 }
        end
          .to(raise_error(RuntimeError, "table is closed"))
      end

      it "requires table to be an Integration::QAWOTable" do
        expect do
          described_class.qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::SINE,
            table: Object.new
          ) { 1.0 }
        end
          .to(
            raise_error(
              TypeError,
              "table must be a Numo::GSL::Integration::QAWOTable"
            )
          )
      end

      it "requires omega to match the QAWO table" do
        expect do
          described_class.qawo(
            0.0,
            Math::PI,
            2.0,
            described_class::SINE,
            table: qawo_table
          ) { 1.0 }
        end
          .to(raise_error(ArgumentError, "omega does not match QAWO table"))
      end

      it "requires interval length to match the QAWO table" do
        expect do
          described_class.qawo(
            0.0,
            Math::PI / 2.0,
            1.0,
            described_class::SINE,
            table: qawo_table
          ) { 1.0 }
        end
          .to(
            raise_error(
              ArgumentError,
              "interval length does not match QAWO table"
            )
          )
      end

      it "requires sine to match the QAWO table" do
        expect do
          described_class.qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::COSINE,
            table: qawo_table
          ) { 1.0 }
        end
          .to(raise_error(ArgumentError, "sine does not match QAWO table"))
      end
    end

    it "integrates with a cosine weight", :aggregate_failures do
      result, error = described_class.qawo(
        0.0,
        Math::PI / 2.0,
        1.0,
        described_class::COSINE
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(1.0))
      expect(error).to(be >= 0.0)
    end

    it "integrates with a sine weight", :aggregate_failures do
      result, error = described_class.qawo(
        0.0,
        Math::PI,
        1.0,
        described_class::SINE
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qawo(
          0.0,
          1.0,
          1.0,
          described_class::COSINE,
          epsabs: 0.0,
          epsrel: 0.0
        ) { 1.0 }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class
        .qawo(
          0.0,
          Math::PI,
          1.0,
          described_class::SINE,
          epsabs: 1e-12,
          epsrel: 1e-12,
          limit: 100
        ) { 1.0 }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(error).to(be >= 0.0)
    end
  end
end
