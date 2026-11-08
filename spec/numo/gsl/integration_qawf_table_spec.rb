# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe "QAWFTable" do
    let(:table_class) { described_class::QAWFTable }
    let(:table_options) do
      {
        omega: 1.0,
        sine: described_class::SINE,
        levels: 50
      }
    end

    describe ".new" do
      it "allocates a table for the QAWF cycle", :aggregate_failures do
        table = table_class.new(**table_options)

        expect(table).to(be_a(described_class::QAWOTable))
        expect(table.omega).to(eq(1.0))
        expect(table.length).to(eq(3.0 * Math::PI))
        expect(table.sine).to(eq(described_class::SINE))
        expect(table.levels).to(eq(50))
        expect(table).not_to(be_closed)
      ensure
        table&.close
      end

      it "rejects reinitializing an open table" do
        table = table_class.new(**table_options)
        expect do
          table.send(:initialize, **table_options)
        end
          .to(raise_error(RuntimeError, "table is already initialized"))
      ensure
        table&.close
      end

      it "calculates the cycle length from omega" do
        table = table_class.new(**table_options, omega: 2.5)

        expect(table.length).to(be_within(1e-15).of(2.0 * Math::PI))
      ensure
        table&.close
      end

      it "calculates the cycle length from the absolute omega" do
        table = table_class.new(**table_options, omega: -1.0)

        expect(table.length).to(eq(3.0 * Math::PI))
      ensure
        table&.close
      end

      it "calculates a finite cycle length for a large omega", :aggregate_failures do
        table = table_class.new(**table_options, omega: Float::MAX / 4.0)
        expect(table.length).to(be_finite)
        expect(table.length).to(be_within(1e-14).of(2.0 * Math::PI))
      ensure
        table&.close
      end

      it "rejects an omega whose cycle length is not finite" do
        expect do
          table_class.new(**table_options, omega: Float::MIN / 2.0)
        end
          .to(
            raise_error(
              ArgumentError,
              "omega produces a non-finite cycle length"
            )
          )
      end

      it "requires omega to be Numeric" do
        expect do
          table_class.new(**table_options, omega: Object.new)
        end
          .to(raise_error(TypeError, "omega must be Numeric"))
      end

      it "requires omega to be finite and nonzero" do
        [0.0, Float::NAN, Float::INFINITY, -Float::INFINITY].each do |omega|
          expect do
            table_class.new(**table_options, omega: omega)
          end
            .to(
              raise_error(
                ArgumentError,
                "omega must be finite and nonzero"
              )
            )
        end
      end

      it "requires sine to be an Integer" do
        expect do
          table_class.new(**table_options, sine: 1.0)
        end
          .to(raise_error(TypeError, "sine must be an Integer"))
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

    describe "use by qawf" do
      let(:table) { table_class.new(**table_options) }
      let(:integrate) do
        lambda do |**options, &block|
          integrand = block || -> (x) { Math.exp(-x) }
          described_class.qawf(
            0.0,
            1.0,
            described_class::SINE,
            table: table,
            **options,
            &integrand
          )
        end
      end

      after do
        table.close unless table.closed?
      end

      it "rejects reentrant use of the same table" do
        expect do
          integrate.call { integrate.call }
        end
          .to(raise_error(RuntimeError, "table is already in use"))
      end

      it "cannot be closed while it is in use" do
        expect do
          integrate.call { table.close }
        end
          .to(raise_error(RuntimeError, "table is already in use"))
      end

      it "can be reused after the integrand raises", :aggregate_failures do
        error_class = Class.new(StandardError)

        expect do
          integrate.call { raise error_class, "failed" }
        end
          .to(raise_error(error_class, "failed"))

        result, error = integrate.call
        expect(result).to(be_within(1e-8).of(0.5))
        expect(error).to(be >= 0.0)
      end

      it "can be reused after GSL raises", :aggregate_failures do
        expect do
          integrate.call(epsabs: 0.0)
        end
          .to(raise_error(Numo::GSL::GslError))

        result, error = integrate.call
        expect(result).to(be_within(1e-8).of(0.5))
        expect(error).to(be >= 0.0)
      end

      it "reuses the same table across calls", :aggregate_failures do
        first_result, first_error = integrate.call
        second_result, second_error = integrate.call

        expect(first_result).to(be_within(1e-8).of(0.5))
        expect(second_result).to(be_within(1e-8).of(0.5))
        expect(first_error).to(be >= 0.0)
        expect(second_error).to(be >= 0.0)
        expect(table).not_to(be_closed)
      end

      it "accepts both workspaces and the table together" do
        workspace = described_class::Workspace.new(limit: 50)
        cycle_workspace = described_class::Workspace.new(limit: 50)

        result, = integrate.call(
          workspace: workspace,
          cycle_workspace: cycle_workspace
        )
        expect(result).to(be_within(1e-8).of(0.5))
      ensure
        workspace&.close
        cycle_workspace&.close
      end

      it "rejects a closed table" do
        table.close

        expect do
          integrate.call
        end
          .to(raise_error(RuntimeError, "table is closed"))
      end

      it "requires omega to match the table" do
        expect do
          described_class.qawf(
            0.0,
            2.0,
            described_class::SINE,
            table: table
          ) { |x| Math.exp(-x) }
        end
          .to(raise_error(ArgumentError, "omega does not match QAWF table"))
      end

      it "requires sine to match the table" do
        expect do
          described_class.qawf(
            0.0,
            1.0,
            described_class::COSINE,
            table: table
          ) { |x| Math.exp(-x) }
        end
          .to(raise_error(ArgumentError, "sine does not match QAWF table"))
      end

      it "preserves the table length after use" do
        length = table.length
        integrate.call

        expect(table.length).to(eq(length))
      end

      it "can use the table with qawo after qawf", :aggregate_failures do
        integrate.call
        result, error = described_class.qawo(
          0.0,
          table.length,
          1.0,
          described_class::SINE,
          table: table
        ) { 1.0 }

        expect(result).to(be_within(1e-10).of(2.0))
        expect(error).to(be >= 0.0)
      end
    end

    describe "use with an invalid table" do
      let(:qawo_table) do
        described_class::QAWOTable.new(
          omega: 1.0,
          length: 3.0 * Math::PI,
          sine: described_class::SINE,
          levels: 50
        )
      end

      after do
        qawo_table.close unless qawo_table.closed?
      end

      it "requires table to be an Integration::QAWFTable" do
        expect do
          described_class.qawf(
            0.0,
            1.0,
            described_class::SINE,
            table: qawo_table
          ) { |x| Math.exp(-x) }
        end
          .to(
            raise_error(
              TypeError,
              "table must be a Numo::GSL::Integration::QAWFTable"
            )
          )
      end
    end
  end
end
