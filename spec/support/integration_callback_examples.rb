# frozen_string_literal: true

RSpec.shared_examples("an integration callback") do
  it "propagates an exception raised by the integrand" do
    error_class = Class.new(StandardError)

    expect do
      integrate.call do
        raise error_class, "integrand failed"
      end
    end
      .to(raise_error(error_class, "integrand failed"))
  end

  it "raises when the integrand returns a non-numeric value" do
    expect do
      integrate.call { Object.new }
    end
      .to(raise_error(TypeError))
  end
end
