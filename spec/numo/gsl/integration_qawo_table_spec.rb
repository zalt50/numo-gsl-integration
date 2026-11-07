# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe "QAWOTable" do
    let(:table_class) { described_class::QAWOTable }
    let(:table_options) do
      {
        omega: 1.0,
        length: Math::PI,
        sine: described_class::SINE,
        levels: 50
      }
    end

    describe ".new" do
      it "allocates a table with fixed parameters", :aggregate_failures do
        table = table_class.new(**table_options)

        expect(table.omega).to(eq(1.0))
        expect(table.length).to(eq(Math::PI))
        expect(table.sine).to(eq(described_class::SINE))
        expect(table.levels).to(eq(50))
        expect(table).not_to(be_closed)
      ensure
        table&.close
      end

      it "requires omega to be Numeric" do
        expect do
          table_class.new(**table_options, omega: Object.new)
        end
          .to(raise_error(TypeError, "omega must be Numeric"))
      end

      it "requires length to be Numeric" do
        expect do
          table_class.new(**table_options, length: Object.new)
        end
          .to(raise_error(TypeError, "length must be Numeric"))
      end

      it "requires a positive length" do
        [0.0, -1.0].each do |length|
          expect do
            table_class.new(**table_options, length: length)
          end
            .to(raise_error(ArgumentError, "length must be positive"))
        end
      end

      it "requires sine to be SINE or COSINE" do
        expect do
          table_class.new(**table_options, sine: -1)
        end
          .to(raise_error(ArgumentError, "sine must be SINE or COSINE"))
      end

      it "requires levels to be an Integer" do
        expect do
          table_class.new(**table_options, levels: 50.0)
        end
          .to(raise_error(TypeError, "levels must be an Integer"))
      end

      it "requires positive levels" do
        [0, -1].each do |levels|
          expect do
            table_class.new(**table_options, levels: levels)
          end
            .to(raise_error(ArgumentError, "levels must be positive"))
        end
      end
    end

    describe ".open" do
      it "yields a table and returns the block result", :aggregate_failures do
        yielded_table = nil
        result = table_class.open(**table_options) do |table|
          yielded_table = table
          :completed
        end

        expect(result).to(eq(:completed))
        expect(yielded_table).to(be_closed)
      end

      it "closes the table when the block raises", :aggregate_failures do
        yielded_table = nil
        error_class = Class.new(StandardError)

        expect do
          table_class.open(**table_options) do |table|
            yielded_table = table
            raise error_class, "failed"
          end
        end
          .to(raise_error(error_class, "failed"))
        expect(yielded_table).to(be_closed)
      end
    end

    describe "#close" do
      it "closes the table" do
        table = table_class.new(**table_options)
        table.close

        expect(table).to(be_closed)
      end

      it "is idempotent", :aggregate_failures do
        table = table_class.new(**table_options)

        expect do
          2.times { table.close }
        end
          .not_to(raise_error)
        expect(table).to(be_closed)
      end
    end

    describe "use by qawo" do
      let(:table) { table_class.new(**table_options) }
      let(:integrate_constant) do
        lambda do |**options|
          described_class.qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::SINE,
            table: table,
            **options
          ) { 1.0 }
        end
      end

      let(:integrate_reentrantly) do
        lambda do
          described_class.qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::SINE,
            table: table
          ) do
            integrate_constant.call
            1.0
          end
        end
      end

      after do
        table.close unless table.closed?
      end

      it "rejects reentrant use of the same table" do
        expect do
          integrate_reentrantly.call
        end
          .to(raise_error(RuntimeError, "table is already in use"))
      end

      it "cannot be closed while it is in use" do
        expect do
          described_class.qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::SINE,
            table: table
          ) do
            table.close
            1.0
          end
        end
          .to(raise_error(RuntimeError, "table is already in use"))
      end

      it "can be reused after the integrand raises", :aggregate_failures do
        error_class = Class.new(StandardError)

        expect do
          described_class.qawo(
            0.0,
            Math::PI,
            1.0,
            described_class::SINE,
            table: table
          ) { raise error_class, "failed" }
        end
          .to(raise_error(error_class, "failed"))
        result, error = integrate_constant.call
        expect(result).to(be_within(1e-10).of(2.0))
        expect(error).to(be >= 0.0)
      end

      it "can be reused after GSL raises", :aggregate_failures do
        expect do
          integrate_constant.call(epsabs: 0.0, epsrel: 0.0)
        end
          .to(raise_error(Numo::GSL::GslError))
        result, error = integrate_constant.call
        expect(result).to(be_within(1e-10).of(2.0))
        expect(error).to(be >= 0.0)
      end
    end

    describe "floating-point parameter matching" do
      let(:table) do
        table_class.new(
          omega: 1.0,
          length: 0.3,
          sine: described_class::SINE,
          levels: 50
        )
      end

      after do
        table.close unless table.closed?
      end

      it(
        "accepts an equivalent rounded interval length",
        :aggregate_failures
      ) do
        result, error = described_class.qawo(
          0.1,
          0.4,
          1.0,
          described_class::SINE,
          table: table
        ) { 1.0 }
        expected = Math.cos(0.1) - Math.cos(0.4)

        expect(result).to(be_within(1e-10).of(expected))
        expect(error).to(be >= 0.0)
      end
    end
  end
end
