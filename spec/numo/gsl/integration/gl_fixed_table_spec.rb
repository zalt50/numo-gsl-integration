# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration::GLFixedTable do
  describe ".new" do
    it "allocates a table with the requested order", :aggregate_failures do
      table = described_class.new(order: 10)

      expect(table.order).to(eq(10))
      expect(table).not_to(be_closed)
    ensure
      table&.close
    end

    it "requires order to be an Integer" do
      expect do
        described_class.new(order: 10.0)
      end
        .to(raise_error(TypeError, "order must be an Integer"))
    end

    it "requires a positive order" do
      [0, -1].each do |order|
        expect do
          described_class.new(order: order)
        end
          .to(raise_error(ArgumentError, "order must be positive"))
      end
    end

    it "rejects reinitializing an open table" do
      table = described_class.new(order: 10)

      expect do
        table.send(:initialize, order: 10)
      end
        .to(raise_error(RuntimeError, "table is already initialized"))
    ensure
      table&.close
    end
  end

  describe ".open" do
    it "yields a table and returns the block result", :aggregate_failures do
      yielded_table = nil
      result = described_class.open(order: 8) do |table|
        yielded_table = table
        :completed
      end

      expect(result).to(eq(:completed))
      expect(yielded_table.order).to(eq(8))
      expect(yielded_table).to(be_closed)
    end

    it "closes the table when the block raises", :aggregate_failures do
      yielded_table = nil
      error_class = Class.new(StandardError)

      expect do
        described_class.open(order: 10) do |table|
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
      table = described_class.new(order: 10)

      table.close

      expect(table).to(be_closed)
    end

    it "is idempotent", :aggregate_failures do
      table = described_class.new(order: 10)

      expect do
        2.times { table.close }
      end
        .not_to(raise_error)
      expect(table).to(be_closed)
    end
  end

  describe "use by glfixed" do
    let(:table) { described_class.new(order: 10) }
    let(:integrate_sine) do
      lambda do |a = 0.0, b = Math::PI, &block|
        integrand = block || -> (x) { Math.sin(x) }
        Numo::GSL::Integration.glfixed(a, b, table: table, &integrand)
      end
    end

    after do
      table.close
    end

    it "rejects reentrant use of the same table" do
      expect do
        integrate_sine.call do |x|
          integrate_sine.call
          Math.sin(x)
        end
      end
        .to(raise_error(RuntimeError, "table is already in use"))
    end

    it "cannot be closed while it is in use" do
      expect do
        integrate_sine.call do |x|
          table.close
          Math.sin(x)
        end
      end
        .to(raise_error(RuntimeError, "table is already in use"))
    end

    it "can be reused after the integrand raises", :aggregate_failures do
      error_class = Class.new(StandardError)

      expect do
        integrate_sine.call { raise error_class, "failed" }
      end
        .to(raise_error(error_class, "failed"))

      expect(integrate_sine.call).to(be_within(1e-10).of(2.0))
    end

    it "reuses the same table across intervals", :aggregate_failures do
      first_result = integrate_sine.call
      second_result = integrate_sine.call(0.0, Math::PI / 2.0)

      expect(first_result).to(be_within(1e-10).of(2.0))
      expect(second_result).to(be_within(1e-10).of(1.0))
      expect(table).not_to(be_closed)
    end
  end
end
