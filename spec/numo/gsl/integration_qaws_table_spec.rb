# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe "QAWSTable" do
    let(:table_class) { described_class::QAWSTable }
    let(:table_options) do
      {
        alpha: -0.5,
        beta: -0.5,
        mu: 0,
        nu: 0
      }
    end

    describe ".new" do
      it "allocates a table with fixed parameters", :aggregate_failures do
        table = table_class.new(**table_options)

        expect(table.alpha).to(eq(-0.5))
        expect(table.beta).to(eq(-0.5))
        expect(table.mu).to(eq(0))
        expect(table.nu).to(eq(0))
        expect(table).not_to(be_closed)
      ensure
        table&.close
      end

      it "requires alpha to be Numeric" do
        expect do
          table_class.new(**table_options, alpha: Object.new)
        end
          .to(raise_error(TypeError, "alpha must be Numeric"))
      end

      it "requires beta to be Numeric" do
        expect do
          table_class.new(**table_options, beta: Object.new)
        end
          .to(raise_error(TypeError, "beta must be Numeric"))
      end

      it "requires alpha to be greater than -1" do
        [-1.0, -2.0].each do |alpha|
          expect do
            table_class.new(**table_options, alpha: alpha)
          end
            .to(raise_error(ArgumentError, "alpha must be greater than -1"))
        end
      end

      it "requires beta to be greater than -1" do
        [-1.0, -2.0].each do |beta|
          expect do
            table_class.new(**table_options, beta: beta)
          end
            .to(raise_error(ArgumentError, "beta must be greater than -1"))
        end
      end

      it "requires mu to be an Integer" do
        expect do
          table_class.new(**table_options, mu: 0.0)
        end
          .to(raise_error(TypeError, "mu must be an Integer"))
      end

      it "requires nu to be an Integer" do
        expect do
          table_class.new(**table_options, nu: 0.0)
        end
          .to(raise_error(TypeError, "nu must be an Integer"))
      end

      it "requires mu to be 0 or 1" do
        expect do
          table_class.new(**table_options, mu: 2)
        end
          .to(raise_error(ArgumentError, "mu must be 0 or 1"))
      end

      it "requires nu to be 0 or 1" do
        expect do
          table_class.new(**table_options, nu: 2)
        end
          .to(raise_error(ArgumentError, "nu must be 0 or 1"))
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

    describe "use by qaws" do
      let(:table) { table_class.new(**table_options) }
      let(:integrate_with_table) do
        lambda do |**options, &block|
          integrand = block || proc { 1.0 }
          described_class.qaws(
            0.0,
            1.0,
            -0.5,
            -0.5,
            0,
            0,
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
          integrate_with_table.call do
            integrate_with_table.call
            1.0
          end
        end
          .to(raise_error(RuntimeError, "table is already in use"))
      end

      it "cannot be closed while it is in use" do
        expect do
          integrate_with_table.call do
            table.close
            1.0
          end
        end
          .to(raise_error(RuntimeError, "table is already in use"))
      end

      it "can be reused after the integrand raises", :aggregate_failures do
        error_class = Class.new(StandardError)

        expect do
          integrate_with_table.call { raise error_class, "failed" }
        end
          .to(raise_error(error_class, "failed"))
        result, error = integrate_with_table.call
        expect(result).to(be_within(1e-10).of(Math::PI))
        expect(error).to(be >= 0.0)
      end

      it "can be reused after GSL raises", :aggregate_failures do
        expect do
          integrate_with_table.call(epsabs: 0.0, epsrel: 0.0)
        end
          .to(raise_error(Numo::GSL::GslError))
        result, error = integrate_with_table.call
        expect(result).to(be_within(1e-10).of(Math::PI))
        expect(error).to(be >= 0.0)
      end
    end
  end
end
