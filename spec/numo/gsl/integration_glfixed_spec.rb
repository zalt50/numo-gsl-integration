# frozen_string_literal: true
RSpec.describe Numo::GSL::Integration do
  describe ".glfixed" do
    let(:integrate) do
      -> (&block) { described_class.glfixed(0.0, 1.0, &block) }
    end

    it_behaves_like("an integration callback")
    it "integrates a smooth function over a finite interval" do
      result = described_class.glfixed(0.0, Math::PI) do |x|
        Math.sin(x)
      end

      expect(result).to(be_within(1e-10).of(2.0))
    end

    it "integrates a polynomial exactly for a sufficient order" do
      result = described_class.glfixed(0.0, 1.0, order: 3) do |x|
        x ** 4
      end

      expect(result).to(be_within(1e-14).of(1.0 / 5.0))
    end

    it "integrates over a non-unit interval" do
      result = described_class.glfixed(-1.0, 2.0, order: 2) do |x|
        x
      end

      expect(result).to(be_within(1e-14).of(1.5))
    end

    it "requires an integer integration order" do
      expect do
        described_class.glfixed(0.0, 1.0, order: 2.5) { |x| x }
      end
        .to(raise_error(TypeError, "order must be an Integer"))
    end

    it "requires a positive integration order" do
      [0, -1].each do |order|
        expect do
          described_class.glfixed(0.0, 1.0, order: order) { |x| x }
        end
          .to(raise_error(ArgumentError, "order must be positive"))
      end
    end

    it "uses order ten when order and table are omitted" do
      result = described_class.glfixed(0.0, 1.0) { |x| x ** 18 }

      expect(result).to(be_within(1e-14).of(1.0 / 19.0))
    end

    context("with a reusable table") do
      let(:table) { described_class::GLFixedTable.new(order: 3) }

      after do
        table.close unless table.closed?
      end

      it "uses the table order when order is omitted" do
        result = described_class.glfixed(
          0.0,
          1.0,
          table: table
        ) { |x| x ** 4 }

        expect(result).to(be_within(1e-14).of(1.0 / 5.0))
      end

      it "accepts an order matching the table" do
        result = described_class.glfixed(
          0.0,
          1.0,
          order: 3,
          table: table
        ) { |x| x ** 4 }

        expect(result).to(be_within(1e-14).of(1.0 / 5.0))
      end

      it "rejects an order that does not match the table" do
        expect do
          described_class.glfixed(
            0.0,
            1.0,
            order: 4,
            table: table
          ) { |x| x ** 4 }
        end
          .to(raise_error(ArgumentError, "order does not match GLFixed table"))
      end

      it "rejects a closed table" do
        table.close

        expect do
          described_class.glfixed(0.0, 1.0, table: table) { |x| x }
        end
          .to(raise_error(RuntimeError, "table is closed"))
      end

      it "requires table to be a GLFixedTable" do
        [nil, Object.new].each do |invalid_table|
          expect do
            described_class.glfixed(
              0.0,
              1.0,
              table: invalid_table
            ) { |x| x }
          end
            .to(
              raise_error(
                TypeError,
                "table must be a Numo::GSL::Integration::GLFixedTable"
              )
            )
        end
      end
    end
  end
end
