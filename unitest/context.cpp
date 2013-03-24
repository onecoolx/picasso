
#include "test.h"

ps_context* get_context()
{
	static ps_canvas* e = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
	static ps_context* c = ps_context_create(e);
	return c;
}

TEST(Context, AntialiasAndGamma)
{
	ps_context * ctx = get_context();
// gamma
	double g = 1.5;
	double old = ps_set_gamma(ctx, g);
	ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

	old = ps_set_gamma(ctx, old);
	EXPECT_EQ(g, old);

	ps_set_antialias(ctx, True);
	ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
	ps_set_antialias(ctx, False);
	ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}




