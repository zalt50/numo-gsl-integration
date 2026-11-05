# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".fixed" do
    let(:integrate) do
      lambda do |&block|
        described_class.fixed(
          :legendre,
          order: 10,
          a: 0.0,
          b: 1.0,
          &block
        )
      end
    end

    it_behaves_like("an integration callback")

    it "integrates a smooth function over a finite interval" do
      result = described_class.fixed(
        :legendre,
        order: 10,
        a: 0.0,
        b: Math::PI
      ) do |x|
        Math.sin(x)
      end

      expect(result).to(be_within(1e-10).of(2.0))
    end

    it "applies the Chebyshev weight over a finite interval" do
      result = described_class.fixed(
        :chebyshev,
        order: 3,
        a: -1.0,
        b: 1.0
      ) do |_x|
        1.0
      end

      expect(result).to(be_within(1e-14).of(Math::PI))
    end

    it "applies the second-kind Chebyshev weight" do
      result = described_class.fixed(
        :chebyshev2,
        order: 3,
        a: -1.0,
        b: 1.0
      ) do |_x|
        1.0
      end

      expect(result).to(be_within(1e-14).of(Math::PI / 2.0))
    end

    it "applies the Gegenbauer weight" do
      result = described_class.fixed(
        :gegenbauer,
        order: 3,
        a: -1.0,
        b: 1.0,
        alpha: 1.0
      ) do |_x|
        1.0
      end

      expect(result).to(be_within(1e-14).of(4.0 / 3.0))
    end

    it "requires alpha for Gegenbauer quadrature" do
      expect do
        described_class.fixed(:gegenbauer, order: 3, a: -1.0, b: 1.0) do |x|
          x
        end
      end
        .to(raise_error(ArgumentError, "alpha is required for gegenbauer"))
    end

    it "applies the Jacobi weight" do
      result = described_class.fixed(
        :jacobi,
        order: 3,
        a: -1.0,
        b: 1.0,
        alpha: 1.0,
        beta: 1.0
      ) do |_x|
        1.0
      end

      expect(result).to(be_within(1e-14).of(4.0 / 3.0))
    end

    it "requires alpha for Jacobi quadrature" do
      expect do
        described_class.fixed(:jacobi, order: 3, a: -1.0, b: 1.0, beta: 1.0) do |x|
          x
        end
      end
        .to(raise_error(ArgumentError, "alpha is required for jacobi"))
    end

    it "requires beta for Jacobi quadrature" do
      expect do
        described_class.fixed(:jacobi, order: 3, a: -1.0, b: 1.0, alpha: 1.0) do |x|
          x
        end
      end
        .to(raise_error(ArgumentError, "beta is required for jacobi"))
    end

    it "applies the Laguerre weight" do
      result = described_class.fixed(
        :laguerre,
        order: 3,
        a: 0.0,
        b: 1.0,
        alpha: 0.0
      ) do |_x|
        1.0
      end

      expect(result).to(be_within(1e-14).of(1.0))
    end

    it "requires alpha for Laguerre quadrature" do
      expect do
        described_class.fixed(:laguerre, order: 3, a: 0.0, b: 1.0) do |x|
          x
        end
      end
        .to(raise_error(ArgumentError, "alpha is required for laguerre"))
    end

    it "applies the Hermite weight" do
      result = described_class.fixed(
        :hermite,
        order: 3,
        a: 0.0,
        b: 1.0,
        alpha: 0.0
      ) do |_x|
        1.0
      end

      expect(result).to(be_within(1e-14).of(Math.sqrt(Math::PI)))
    end

    it "requires alpha for Hermite quadrature" do
      expect do
        described_class.fixed(:hermite, order: 3, a: 0.0, b: 1.0) do |x|
          x
        end
      end
        .to(raise_error(ArgumentError, "alpha is required for hermite"))
    end

    it "applies the exponential weight" do
      result = described_class.fixed(
        :exponential,
        order: 3,
        a: -1.0,
        b: 1.0,
        alpha: 1.0
      ) do |_x|
        1.0
      end

      expect(result).to(be_within(1e-14).of(1.0))
    end

    it "requires alpha for exponential quadrature" do
      expect do
        described_class.fixed(:exponential, order: 3, a: -1.0, b: 1.0) do |x|
          x
        end
      end
        .to(raise_error(ArgumentError, "alpha is required for exponential"))
    end

    it "integrates a polynomial exactly for a sufficient order" do
      result = described_class.fixed(
        :legendre,
        order: 3,
        a: 0.0,
        b: 1.0
      ) do |x|
        x ** 4
      end

      expect(result).to(be_within(1e-14).of(1.0 / 5.0))
    end

    it "integrates over a non-unit interval" do
      result = described_class.fixed(
        :legendre,
        order: 2,
        a: -1.0,
        b: 2.0
      ) do |x|
        x
      end

      expect(result).to(be_within(1e-14).of(1.5))
    end

    it "matches glfixed for the same order and interval" do
      fixed_result = described_class.fixed(
        :legendre,
        order: 5,
        a: -2.0,
        b: 3.0
      ) do |x|
        Math.exp(x)
      end

      glfixed_result = described_class.glfixed(-2.0, 3.0, order: 5) do |x|
        Math.exp(x)
      end

      expect(fixed_result).to(be_within(1e-12).of(glfixed_result))
    end

    it "rejects an unsupported quadrature type" do
      expect do
        described_class.fixed(:rational, order: 3, a: 0.0, b: 1.0) do |x|
          x
        end
      end
        .to(
          raise_error(
            ArgumentError,
            "unsupported fixed quadrature type: rational"
          )
        )
    end

    it "requires the quadrature type to be a Symbol" do
      expect do
        described_class.fixed(
          "legendre",
          order: 3,
          a: 0.0,
          b: 1.0
        ) do |x|
          x
        end
      end
        .to(raise_error(TypeError, "type must be a Symbol"))
    end

    it "requires an integer integration order" do
      expect do
        described_class.fixed(
          :legendre,
          order: 2.5,
          a: 0.0,
          b: 1.0
        ) do |x|
          x
        end
      end
        .to(raise_error(TypeError, "order must be an Integer"))
    end

    it "requires a positive integration order" do
      [0, -1].each do |order|
        expect do
          described_class.fixed(
            :legendre,
            order: order,
            a: 0.0,
            b: 1.0
          ) do |x|
            x
          end
        end
          .to(raise_error(ArgumentError, "order must be positive"))
      end
    end
  end
end
