/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.net
 *
 * Copyright 2011 Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CUnit/Basic.h"

#include "liblwgeom_internal.h"
#include "lwgeodetic.h"
#include "lwgeodetic_tree.h"
#include "cu_tester.h"


static void test_tree_circ_create(void)
{
	LWLINE *g;
	CIRC_NODE *c;
	/* Line with 4 edges */
	g = lwgeom_as_lwline(lwgeom_from_wkt("LINESTRING(0 88,0 89,0 90,180 89,180 88)", LW_PARSER_CHECK_NONE));
	c = circ_tree_new(g->points);
	//circ_tree_print(c, 0);

	if ( CIRC_NODE_SIZE > 4 )
	{
		CU_ASSERT(c->num_nodes == 4);
	}
	else
	{
		CU_ASSERT(c->num_nodes  == ( 4 % CIRC_NODE_SIZE ? 1 : 0 ) + 4 / CIRC_NODE_SIZE);
	}
		
	circ_tree_free(c);
	lwline_free(g);
}


static void test_tree_circ_pip(void)
{
	LWLINE *g;
	CIRC_NODE *c;
	POINT2D pt, pt_outside;
	int rv, on_boundary;
	
	pt.x = 0.0;
	pt.y = 0.0;
	pt_outside.x = -2.0;
	pt_outside.y = 0.0;
	
	/* Point in square */
	g = lwgeom_as_lwline(lwgeom_from_wkt("LINESTRING(-1 -1,1 -1,1 1,-1 1,-1 -1)", LW_PARSER_CHECK_NONE));
	c = circ_tree_new(g->points);
	rv = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	CU_ASSERT_EQUAL(rv, 1);

	/* Point on other side of square */
	pt.x = 2.0;
	pt.y = 0.0;
	rv = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	CU_ASSERT_EQUAL(rv, 0);

	/* Clean and do new shape */
	circ_tree_free(c);
	lwline_free(g);

	/* Point in square, stab passing through vertex */
	pt.x = 0.0;
	pt.y = 0.0;
	g = lwgeom_as_lwline(lwgeom_from_wkt("LINESTRING(-1 -1,0 -1,1 -1,1 0,1 1,0 1,-1 1,-1 0,-1 -1)", LW_PARSER_CHECK_NONE));
	c = circ_tree_new(g->points);
	//circ_tree_print(c, 0);
	rv = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	CU_ASSERT_EQUAL(rv, 1);

	/* Point on other side of square, stab passing through vertex */
	pt.x = 2.0;
	pt.y = 0.0;
	rv = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	CU_ASSERT_EQUAL(rv, 0);

	/* Clean and do new shape */
	circ_tree_free(c);
	lwline_free(g);

	/* Point outside "w" thing, stab passing through vertexes and grazing pointy thing */
	pt.x = 2.0;
	pt.y = 0.0;
	g = lwgeom_as_lwline(lwgeom_from_wkt("LINESTRING(-1 -1,0 -1,1 -1,1 0,1 1,0 0,-1 1,-1 0,-1 -1)", LW_PARSER_CHECK_NONE));
	c = circ_tree_new(g->points);
	//circ_tree_print(c, 0);
	rv = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	//printf("rv %d\n", rv);
	CU_ASSERT_EQUAL(rv, 0);

	/* Point inside "w" thing, stab passing through vertexes and grazing pointy thing */
	pt.x = 0.8;
	pt.y = 0.0;
	rv = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	//printf("rv %d\n", rv);
	CU_ASSERT_EQUAL(rv, 1);

	/* Clean and do new shape */
	circ_tree_free(c);
	lwline_free(g);

}

static void test_tree_circ_pip2(void)
{
	LWGEOM* g;
	LWPOLY* p;
	LWPOINT* lwpt;
	int rv_classic, rv_tree, on_boundary;
	POINT2D pt, pt_outside;
	GBOX gbox;
	CIRC_NODE *c;

	g = lwgeom_from_wkt("POLYGON((0 0,0 1,1 1,1 0,0 0))", LW_PARSER_CHECK_NONE);
	p = lwgeom_as_lwpoly(g);

	pt.x = 0.2;
	pt.y = 0.1;
	lwgeom_calculate_gbox_geodetic(g, &gbox);
	gbox_pt_outside(&gbox, &pt_outside);
	c = circ_tree_new(p->rings[0]);
	//circ_tree_print(c, 0);
	rv_classic = lwpoly_covers_point2d(p, &pt);
	rv_tree = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	CU_ASSERT_EQUAL(rv_tree, rv_classic);
	circ_tree_free(c);
	lwgeom_free(g);
	
	g = lwgeom_from_hexwkb("0103000020E6100000010000004700000000000000621119C000000040C70C4B40000000E0CC6C18C0000000A026FF4A4000000040438519C000000000E8F44A40000000000F5318C00000004024C84A4000000060F9E518C0000000A027AD4A40000000805E0D18C0000000C0F5784A4000000040539718C000000080815E4A40000000C026FE19C0000000C0502D4A4000000060127019C000000040EA164A40000000003BFD1BC0000000609E234A4000000080D9011CC000000060B9114A4000000040C8501EC0000000C0D50C4A40000000C05F2C20C000000040C9E749400000006008D820C000000080D6F0494000000080139F20C000000060F3DE4940000000A0B16421C0000000C059C94940000000808FA223C0000000C007B949400000000041E722C000000080C3DC4940000000808F4224C0000000405DCE494000000060752923C000000040A9EF4940000000005CAD24C0000000C036E4494000000040F88624C00000008078FE494000000060558523C00000006025134A40000000403AED24C00000000011174A40000000A05E7D23C0000000E0A41F4A4000000040F0AD23C0000000809A304A4000000040A64E23C000000040C9474A40000000C0FCA221C0000000C030554A40000000805EDD23C0000000E010474A4000000040BFF822C00000008078664A4000000080C98F22C000000040E2914A40000000C036E021C00000002024924A4000000080D9E121C000000000D0A14A4000000040533723C000000040B99D4A40000000204B1E23C0000000C0CCB04A4000000000625A24C0000000A071B44A40000000004A5F24C0000000806FC64A40000000E0DD6523C00000006088CC4A400000004012D023C0000000001AE14A40000000806E1F23C0000000400BEE4A40000000E0A2E123C0000000C017EF4A4000000060449423C00000004003F94A40000000C0DC0624C0000000A0ED1B4B40000000A0803F24C0000000005D0C4B4000000040753924C000000080701D4B400000002021F320C00000000001234B4000000000C65221C000000080792D4B40000000406D6020C0000000001A514B4000000040BF9821C0000000A00E594B400000000031B520C0000000C0726B4B400000002019EA20C000000020977F4B400000000002ED1FC0000000E0B49B4B400000000084CC1EC0000000602D8C4B4000000020BB2A1FC000000060239B4B4000000040AE871EC0000000A0FDA14B400000008077771EC0000000C0E5864B40000000C0AABA1EC000000000B7794B40000000C03EC91DC0000000E020874B4000000000A4301EC0000000C0C49B4B4000000000B5811DC0000000A0A3B04B400000004095BD1BC000000020869E4B400000004091021DC00000004009894B40000000409D361EC000000080A2614B40000000809FB41FC0000000A0AB594B40000000C046021FC0000000C0164C4B40000000C0EC5020C0000000E05A384B4000000040DF3C1EC0000000803F104B4000000000B4221DC0000000C0CD0F4B40000000C0261E1CC00000006067354B4000000080E17A1AC000000080C3044B4000000000621119C000000040C70C4B40", LW_PARSER_CHECK_NONE);
	p = lwgeom_as_lwpoly(g);
	lwpt = (LWPOINT*)lwgeom_from_hexwkb("0101000020E610000057B89C28FEB320C09C8102CB3B2B4A40", LW_PARSER_CHECK_NONE);
	lwpoint_getPoint2d_p(lwpt, &pt);
	lwgeom_calculate_gbox_geodetic(g, &gbox);
	gbox_pt_outside(&gbox, &pt_outside);
	//printf("OUTSIDE POINT(%g %g)\n", pt_outside.x, pt_outside.y);
	c = circ_tree_new(p->rings[0]);
	rv_classic = lwpoly_covers_point2d(p, &pt);
	rv_tree = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	CU_ASSERT_EQUAL(rv_tree, rv_classic);
	circ_tree_free(c);
	lwpoint_free(lwpt);
	lwgeom_free(g);

	g = lwgeom_from_hexwkb("0103000020E610000001000000CF0100000000000000004EC03943F5FFFF7F56400000000000003E403943F5FFFF7F56400000000000003E401842CEFBFF7F56400000000000003E402F849CF7FF7F56400000000000003E4047C66AF3FF7F56400000000000003E405F0839EFFF7F56400000000000003E40774A07EBFF7F56400000000000003E408F8CD5E6FF7F56400000000000003E40A6CEA3E2FF7F56400000000000003E40D65240DAFF7F56400000000000003E4006D7DCD1FF7F56400000000000003E40355B79C9FF7F56400000000000003E407D21E4BCFF7F56400000000000003E40DC291DACFF7F56400000000000003E403B32569BFF7F56400000000000003E40CABE2B82FF7F56400000000000003E40594B0169FF7F56400000000000003E40309E4143FF7F56400000000000003E401E335019FF7F56400000000000003E403C4CFBE6FE7F56400000000000003E40B96DDFA3FE7F56400000000000003E407E552E54FE7F56400000000000003E40A245B6F3FD7F56400000000000003E403D80457EFD7F56400000000000003E407E8978EBFC7F56400000000000003E407FA31D37FC7F56400000000000003E405510035DFB7F56400000000000003E404A969350FA7F56400000000000003E40BC3D0801F97F56400000000000003E40C3482F6AF77F56400000000000003E40D5011077F57F56400000000000003E406CB3B112F37F56400000000000003E402C2CB81FF07F56400000000000003E40A4F8F884EC7F56400000000000003E40C5AD8218E87F56400000000000003E40C1A6CEA3E27F56400000000000003E40A1BAB9F8DB7F56400000000000003E40401361C3D37F56400000000000003E40639813B4C97F56400000000000003E408D429259BD7F56400000000000003E40B854A52DAE7F56400000000000003E406F9EEA909B7F56400000000000003E403FC6DCB5847F56400000000000003E408FC536A9687F56400000000000003E402975C938467F56400000000000003E403F8D7BF31B7F56400000000000003E40F4311F10E87E56400000000000003E40C1FBAA5CA87E56400000000000003E40D2FF722D5A7E56400000000000003E4009A7052FFA7D56400000000000003E403332C85D847D56400000000000003E40B35F77BAF37C56400000000000003E40253ACB2C427C56400000000000003E40FA7B293C687B56400000000000003E407D3D5FB35C7A56400000000000003E40E3A25A44147956400000000000003E40A0504F1F817756400000000000003E4062855B3E927556400000000000003E40B62BF4C1327356400000000000003E40280D350A497056400000000000003E4061DC0DA2B56C56400000000000003E40B81E85EB516856400000000000003E403237DF88EE6256400000000000003E4041EE224C515C56400000000000003E409EE925C6325456400000000000003E400B2593533B4A56400000000000003E4089CF9D60FF3D56400000000000003E40F04A92E7FA2E56400000000000003E40AC3594DA8B1C56400000000000003E40C9022670EB0556400000000000003E4069A510C825EA55400000000000003E4033DC80CF0FC855400000000000003E40FF907EFB3A9E55400000000000003E404203B16CE66A55400000000000003E40DA39CD02ED2B55400000000000003E4070404B57B0DE54400000000000003E4000000000008054403333333333B33D4000000000008054406666666666663D4000000000008054409999999999193D400000000000805440CCCCCCCCCCCC3C4000000000008054400000000000803C4000000000008054403333333333333C4000000000008054406666666666E63B4000000000008054409999999999993B400000000000805440CCCCCCCCCC4C3B4000000000008054400000000000003B4000000000008054403333333333B33A4000000000008054406666666666663A4000000000008054409999999999193A400000000000805440CCCCCCCCCCCC3940000000000080544000000000008039400000000000805440333333333333394000000000008054406666666666E63840000000000080544099999999999938400000000000805440CCCCCCCCCC4C38400000000000805440000000000000384000000000008054403333333333B3374000000000008054406666666666663740000000000080544099999999991937400000000000805440CDCCCCCCCCCC3640000000000080544000000000008036400000000000805440333333333333364000000000008054406666666666E63540000000000080544099999999999935400000000000805440CDCCCCCCCC4C35400000000000805440000000000000354000000000008054403333333333B3344000000000008054406666666666663440000000000080544099999999991934400000000000805440CDCCCCCCCCCC3340000000000080544000000000008033400000000000805440333333333333334000000000008054406666666666E63240000000000080544099999999999932400000000000805440CDCCCCCCCC4C32400000000000805440000000000000324000000000008054403333333333B3314000000000008054406666666666663140000000000080544099999999991931400000000000805440CDCCCCCCCCCC304000000000008054400000000000803040000000000080544033333333333330400000000000805440CCCCCCCCCCCC2F4000000000008054403333333333332F4000000000008054409999999999992E4000000000008054400000000000002E4000000000008054406666666666662D400000000000805440CCCCCCCCCCCC2C4000000000008054403333333333332C4000000000008054409999999999992B4000000000008054400000000000002B4000000000008054406666666666662A400000000000805440CCCCCCCCCCCC2940000000000080544033333333333329400000000000805440999999999999284000000000008054400000000000002840000000000080544066666666666627400000000000805440CDCCCCCCCCCC2640000000000080544033333333333326400000000000805440999999999999254000000000008054400000000000002540000000000080544066666666666624400000000000805440CDCCCCCCCCCC2340000000000080544033333333333323400000000000805440999999999999224000000000008054400000000000002240000000000080544066666666666621400000000000805440CDCCCCCCCCCC20400000000000805440333333333333204000000000008054403333333333331F4000000000008054400000000000001E400000000000805440CCCCCCCCCCCC1C4000000000008054409999999999991B4000000000008054406666666666661A4000000000008054403333333333331940000000000080544000000000000018400000000000805440CDCCCCCCCCCC1640000000000080544099999999999915400000000000805440666666666666144000000000008054403333333333331340000000000080544000000000000012400000000000805440CDCCCCCCCCCC104000000000008054403333333333330F400000000000805440CCCCCCCCCCCC0C4000000000008054406666666666660A400000000000805440000000000000084000000000008054409999999999990540000000000080544033333333333303400000000000805440CDCCCCCCCCCC00400000000000805440CCCCCCCCCCCCFC3F0000000000805440000000000000F83F0000000000805440333333333333F33F0000000000805440CCCCCCCCCCCCEC3F0000000000805440333333333333E33F0000000000805440333333333333D33F00000000008054400000000000000000000000000080544000000000000000002174D0251C7C54400000000000000000F96871C6307854400000000000000000E6E61BD13D745440000000000000000019726C3D437054400000000000000000F0129CFA406C54400000000000000000CCD1E3F7366854400000000000000000F374AE28256454400000000000000000ACC266800B6054400000000000000000700514EAE95B544000000000000000006EC1525DC057544000000000000000001C412AC58E5354400000000000000000AB083719554F5440000000000000000091628044134B544000000000000000001615713AC94654400000000000000000992842EA7642544000000000000000007AA52C431C3E5440000000000000000000529B38B939544000000000000000008B36C7B94D3554400000000000000000795BE9B5D9305440000000000000000029C93A1C5D2C54400000000000000000E54526E0D72754400000000000000000211CB3EC49235440000000000000000027124C35B31E5440000000000000000055302AA9131A544000000000000000000A7F86376B1554400000000000000000BE4868CBB91054400000000000000000A1116C5CFF0B54400000000000000000406667D13B0754400000000000000000E50CC51D6F0254400000000000000000ED0DBE3099FD53400000000000000000D0B359F5B9F853400000000000000000D6C4025FD1F353400000000000000000768BC058DFEE534000000000000000000E10CCD1E3E953400000000000000000FF5A5EB9DEE453400000000000000000A774B0FECFDF534000000000000000006665FB90B7DA53400000000000000000CBB9145795D55340000000000000000005F6984869D053400000000000000000BCE82B4833CB534000000000000000001E166A4DF3C553400000000000000000A3C85A43A9C053400000000000000000DA8CD31055BB53400000000000000000F3E670ADF6B5534000000000000000007C6308008EB053400000000000000000EC4CA1F31AAB534000000000000000008B69A67B9DA553400000000000000000E945ED7E15A0534000000000000000004CA8E0F0829A53400000000000000000431D56B8E5945340000000000000000045EF54C03D8F534000000000000000009BE447FC8A8953400000000000000000D2890453CD83534000000000000000004BE7C3B3047E5340000000000000000093895B053178534000000000000000000B79043752725340000000000000000012BEF737686C5340000000000000000036E50AEF726653400000000000000000EF3845477260534000000000000000009CC1DF2F665A53400000000000000000CC0BB08F4E5453400000000000000000DE1FEF552B4E53400000000000000000618A7269FC4753400000000000000000B45373B9C141534000000000000000006708C72C7B3B53400000000000000000D9B0A6B228355340000000000000000098D9E731CA2E53400000000000000000340F60915F2853400000000000000000F4177AC4E821534000000000000000007EC2D9AD651B534000000000000000003317B83CD61453400000000000000000A0A2EA573A0E5340000000000000000056F146E6910753400000000000000000B20B06D7DC0053400000000000000000457EFD101BFA524000000000000000008593347F4CF352400000000000000000191A4F0471EC5240000000000000000049D8B79388E552400000000000000000BA9C121093DE52400000000000000000E6B1666490D75240000000000000000059A4897780D0524000000000000000008CBE823463C9524000000000000000000D8D278238C2524000000000000000006B9C4D4700BB524000000000000000001D37FC6EBAB352400000000000000000B3E908E066AC52400000000000000000A4FE7A8505A5524000000000000000009544F641969D52400000000000000000FF05820019965240000000000000000070CFF3A78D8E52400000000000000000772D211FF486524000000000000000008B6A11514C7F52400000000000000000545568209677524000000000000000005F7AFB73D16F524000000000000000002424D236FE675240000000000000000033DFC14F1C6052400000000000000000317A6EA12B5852400000000000000000963FDF162C505240000000000000000008FEB7921D48524000000000000000000000000000405240999999999999C9BF0000000000405240999999999999D9BF0000000000405240333333333333E3BF0000000000405240999999999999E9BF0000000000405240000000000000F0BF0000000000405240333333333333F3BF0000000000405240666666666666F6BF0000000000405240999999999999F9BF0000000000405240CCCCCCCCCCCCFCBF000000000040524000000000000000C0000000000040524099999999999901C0000000000040524033333333333303C00000000000405240CDCCCCCCCCCC04C0000000000040524066666666666606C0000000000040524000000000000008C0000000000040524099999999999909C000000000004052403333333333330BC00000000000405240CCCCCCCCCCCC0CC000000000004052406666666666660EC0000000000040524000000000000010C00000000000405240CDCCCCCCCCCC10C0000000000040524099999999999911C0000000000040524066666666666612C0000000000040524033333333333313C0000000000040524000000000000014C00000000000405240CDCCCCCCCCCC14C0000000000040524099999999999915C0000000000040524066666666666616C0000000000040524033333333333317C0000000000040524000000000000018C00000000000405240CCCCCCCCCCCC18C0000000000040524099999999999919C000000000004052406666666666661AC000000000004052403333333333331BC000000000004052400000000000001CC00000000000405240CCCCCCCCCCCC1CC000000000004052409999999999991DC000000000004052406666666666661EC000000000004052403333333333331FC0000000000040524000000000000020C0000000000040524066666666666620C00000000000405240CDCCCCCCCCCC20C0000000000040524033333333333321C0000000000040524099999999999921C0000000000040524000000000000022C0000000000040524066666666666622C00000000000405240CDCCCCCCCCCC22C0000000000040524033333333333323C0000000000040524099999999999923C0000000000040524000000000000024C0000000000040524066666666666624C00000000000405240CDCCCCCCCCCC24C0000000000040524033333333333325C0000000000040524099999999999925C0000000000040524000000000000026C0000000000040524066666666666626C00000000000405240CDCCCCCCCCCC26C0000000000040524033333333333327C0000000000040524099999999999927C0000000000040524000000000000028C0000000000040524066666666666628C00000000000405240CCCCCCCCCCCC28C0000000000040524033333333333329C0000000000040524099999999999929C000000000004052400000000000002AC000000000004052406666666666662AC00000000000405240CCCCCCCCCCCC2AC000000000004052403333333333332BC000000000004052409999999999992BC000000000004052400000000000002CC000000000004052406666666666662CC00000000000405240CCCCCCCCCCCC2CC000000000004052403333333333332DC000000000004052409999999999992DC000000000004052400000000000002EC000000000004052406666666666662EC00000000000405240CCCCCCCCCCCC2EC000000000004052403333333333332FC000000000004052409999999999992FC0000000000040524000000000000030C0000000000040524033333333333330C0000000000040524066666666666630C0000000000040524099999999999930C00000000000405240CDCCCCCCCCCC30C0000000000040524000000000000031C0000000000040524033333333333331C0000000000040524066666666666631C0000000000040524099999999999931C00000000000405240CDCCCCCCCCCC31C0000000000040524000000000000032C0000000000040524033333333333332C0000000000040524066666666666632C0000000000040524099999999999932C00000000000405240CDCCCCCCCCCC32C0000000000040524000000000000033C0000000000040524033333333333333C0000000000040524066666666666633C0000000000040524099999999999933C00000000000405240CDCCCCCCCCCC33C0000000000040524000000000000034C0000000000040524000000000000034C0000000000080514000000000008043C00000000000C04F4000000000008045C00000000000404D4030116F9D7F8B45C00000000000404D408FA67A32FF9645C00000000000404D40BFB7E9CF7EA245C00000000000404D401E4DF564FEAD45C00000000000404D404E5E64027EB945C00000000000404D40AEF36F97FDC445C00000000000404D40DD04DF347DD045C00000000000404D403D9AEAC9FCDB45C00000000000404D406DAB59677CE745C00000000000404D40CC4065FCFBF245C00000000000404D40FC51D4997BFE45C00000000000404D405BE7DF2EFB0946C00000000000404D408BF84ECC7A1546C00000000000404D40EB8D5A61FA2046C00000000000404D401B9FC9FE792C46C00000000000404D407A34D593F93746C00000000000404D40AA454431794346C00000000000404D40DA56B3CEF84E46C00000000000404D4039ECBE63785A46C00000000000404D4069FD2D01F86546C00000000000404D40C8923996777146C00000000000404D40F8A3A833F77C46C00000000000404D405839B4C8768846C00000000000404D40884A2366F69346C00000000000404D40E7DF2EFB759F46C00000000000404D4017F19D98F5AA46C00000000000404D407686A92D75B646C00000000000404D40A69718CBF4C146C00000000000404D40062D246074CD46C00000000000404D40353E93FDF3D846C00000000000404D4095D39E9273E446C00000000000404D40C5E40D30F3EF46C00000000000404D40247A19C572FB46C00000000000404D40548B8862F20647C00000000000404D40B42094F7711247C00000000000404D40E3310395F11D47C00000000000404D4013437232712947C00000000000404D4073D87DC7F03447C00000000000404D40A2E9EC64704047C00000000000404D40027FF8F9EF4B47C00000000000404D40329067976F5747C00000000000404D409125732CEF6247C00000000000404D40C136E2C96E6E47C00000000000404D4021CCED5EEE7947C00000000000404D4050DD5CFC6D8547C00000000000404D40B0726891ED9047C00000000000404D40E083D72E6D9C47C00000000000404D403F19E3C3ECA747C00000000000404D406F2A52616CB347C00000000000404D40CEBF5DF6EBBE47C00000000000404D40FED0CC936BCA47C00000000000404D405E66D828EBD547C00000000000404D408E7747C66AE147C00000000000404D40BD88B663EAEC47C00000000000404D401D1EC2F869F847C00000000000404D404D2F3196E90348C00000000000404D40ACC43C2B690F48C00000000000404D40DCD5ABC8E81A48C00000000000404D403B6BB75D682648C00000000000404D406B7C26FBE73148C00000000000404D40CB113290673D48C00000000000404D40FB22A12DE74848C00000000000404D405AB8ACC2665448C00000000000404D408AC91B60E65F48C00000000000404D40E95E27F5656B48C00000000000404D4019709692E57648C00000000000404D407905A227658248C00000000000404D40A81611C5E48D48C00000000000404D4008AC1C5A649948C00000000000404D4038BD8BF7E3A448C00000000000404D4068CEFA9463B048C00000000000404D40C763062AE3BB48C00000000000404D40F77475C762C748C00000000000404D40560A815CE2D248C00000000000404D40861BF0F961DE48C00000000000404D40E6B0FB8EE1E948C00000000000404D4015C26A2C61F548C00000000000404D4000000000000049C00000000000404D400000000000E04CC0000000000040504000000000000053C000000000000053400000000000C052C000000000008053400000000000004EC000000000008054400000000000004EC03943F5FFFF7F5640", LW_PARSER_CHECK_NONE);
	p = lwgeom_as_lwpoly(g);
	lwpt = (LWPOINT*)lwgeom_from_hexwkb("0101000020E610000031B1F9B836A046C03C889D2974124E40", LW_PARSER_CHECK_NONE);
	lwpoint_getPoint2d_p(lwpt, &pt);
	lwgeom_calculate_gbox_geodetic(g, &gbox);
	gbox_pt_outside(&gbox, &pt_outside);
	//printf("OUTSIDE POINT(%g %g)\n", pt_outside.x, pt_outside.y);
	c = circ_tree_new(p->rings[0]);
	rv_classic = lwpoly_covers_point2d(p, &pt);
	rv_tree = circ_tree_contains_point(c, &pt, &pt_outside, &on_boundary);
	CU_ASSERT_EQUAL(rv_tree, rv_classic);
	circ_tree_free(c);
	lwpoint_free(lwpt);
	lwgeom_free(g);

}


static void test_tree_circ_distance(void)
{
	LWGEOM *lwg1, *lwg2;
	CIRC_NODE *c1, *c2;
	SPHEROID s;
	double d1, d2, d3, d4;
	double threshold = 0.0;
	
	spheroid_init(&s, 1.0, 1.0);

	/* Ticket #1958 */
	lwg1 = lwgeom_from_wkt("LINESTRING(22.88333 41.96667,21.32667 42.13667)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("POLYGON((22.94472 41.34667,22.87528 41.99028,22.87389 41.98472,22.87472 41.98333,22.94472 41.34667))", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
	d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
//	printf("d1 = %g   d2 = %g\n", d1 * WGS84_RADIUS, d2 * WGS84_RADIUS);
//	printf("line\n");
//	circ_tree_print(c1, 0);
//	printf("poly\n");
//	circ_tree_print(c2, 0);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.0000001);
	
	/* Ticket #1951 */
	lwg1 = lwgeom_from_wkt("LINESTRING(0 0, 0 0)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("POINT(0.1 0.1)", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
	d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.00001);

	lwg1 = lwgeom_from_wkt("LINESTRING(-1 -1,0 -1,1 -1,1 0,1 1,0 0,-1 1,-1 0,-1 -1)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("POINT(-2 0)", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
	d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.00001);

	lwg1 = lwgeom_from_wkt("LINESTRING(-1 -1,0 -1,1 -1,1 0,1 1,0 0,-1 1,-1 0,-1 -1)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("POINT(2 2)", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
	d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.00001);

	lwg1 = lwgeom_from_wkt("LINESTRING(-1 -1,0 -1,1 -1,1 0,1 1,0 0,-1 1,-1 0,-1 -1)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("POINT(1 1)", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
	d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.00001);

	lwg1 = lwgeom_from_wkt("LINESTRING(-1 -1,0 -1,1 -1,1 0,1 1,0 0,-1 1,-1 0,-1 -1)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("POINT(1 0.5)", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
	d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
//	printf("distance_tree %g\n", distance_tree);
//	printf("distance_geom %g\n", distance_geom);
//	circ_tree_print(cline, 0);
//	circ_tree_print(cpoint, 0);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.00001);


	/* Ticket #2351 */
	lwg1 = lwgeom_from_wkt("LINESTRING(149.386990599235 -26.3567415843982,149.386990599247 -26.3567415843965)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("POINT(149.386990599235 -26.3567415843982)", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
	d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
//	printf("d1 = %g   d2 = %g\n", d1 * WGS84_RADIUS, d2 * WGS84_RADIUS);
//	printf("line\n");
//	circ_tree_print(c1, 0);
//	printf("point\n");
//	circ_tree_print(c2, 0);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.0000001);
	
	/* Ticket #2634 */
	lwg1 = lwgeom_from_wkt("MULTIPOINT (-10 40,-10 65,10 40,10 65,30 40,30 65,50 40,50 65)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("POLYGON((-9.1111111 40,-9.14954053919354 39.6098193559677,-9.26335203497743 39.2346331352698,-9.44817187539491 38.8888595339608,-9.6968975376269 38.5857864376269,-9.99997063396079 38.3370607753949,-10.3457442352698 38.1522409349774,-10.7209304559677 38.0384294391935,-11.1111111 38,-11.5012917440323 38.0384294391935,-11.8764779647302 38.1522409349774,-12.2222515660392 38.3370607753949,-12.5253246623731 38.5857864376269,-12.7740503246051 38.8888595339608,-12.9588701650226 39.2346331352698,-13.0726816608065 39.6098193559677,-13.1111111 40,-13.0726816608065 40.3901806440322,-12.9588701650226 40.7653668647302,-12.7740503246051 41.1111404660392,-12.5253246623731 41.4142135623731,-12.2222515660392 41.6629392246051,-11.8764779647302 41.8477590650226,-11.5012917440323 41.9615705608065,-11.1111111 42,-10.7209304559678 41.9615705608065,-10.3457442352698 41.8477590650226,-9.9999706339608 41.6629392246051,-9.69689753762691 41.4142135623731,-9.44817187539491 41.1111404660392,-9.26335203497743 40.7653668647302,-9.14954053919354 40.3901806440323,-9.1111111 40))", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
	d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
//	printf("d1 = %g   d2 = %g\n", d1 * WGS84_RADIUS, d2 * WGS84_RADIUS);
//	printf("multipoint\n");
//	circ_tree_print(c1, 0);
//	printf("polygon\n");
//	circ_tree_print(c2, 0);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.0000001);

	/* Ticket #2634 */
	lwg1 = lwgeom_from_wkt("MULTIPOINT Z (-10 40 1,-10 65 1,10 40 1,10 65 1,30 40 1,30 65 1,50 40 1,50 65 1,-10 40 2,-10 65 2,10 40 2,10 65 2,30 40 2,30 65 2,50 40 2,50 65 2,-10 40 3,-10 65 3,10 40 3,10 65 3,30 40 3,30 65 3,50 40 3,50 65 3)", LW_PARSER_CHECK_NONE);
	lwg2 = lwgeom_from_wkt("MULTIPOLYGON(((-9.1111111 40,-9.14954053919354 39.6098193559677,-9.26335203497743 39.2346331352698,-9.44817187539491 38.8888595339608,-9.6968975376269 38.5857864376269,-9.99997063396079 38.3370607753949,-10.3457442352698 38.1522409349774,-10.7209304559677 38.0384294391935,-11.1111111 38,-11.5012917440323 38.0384294391935,-11.8764779647302 38.1522409349774,-12.2222515660392 38.3370607753949,-12.5253246623731 38.5857864376269,-12.7740503246051 38.8888595339608,-12.9588701650226 39.2346331352698,-13.0726816608065 39.6098193559677,-13.1111111 40,-13.0726816608065 40.3901806440322,-12.9588701650226 40.7653668647302,-12.7740503246051 41.1111404660392,-12.5253246623731 41.4142135623731,-12.2222515660392 41.6629392246051,-11.8764779647302 41.8477590650226,-11.5012917440323 41.9615705608065,-11.1111111 42,-10.7209304559678 41.9615705608065,-10.3457442352698 41.8477590650226,-9.9999706339608 41.6629392246051,-9.69689753762691 41.4142135623731,-9.44817187539491 41.1111404660392,-9.26335203497743 40.7653668647302,-9.14954053919354 40.3901806440323,-9.1111111 40)))", LW_PARSER_CHECK_NONE);
	c1 = lwgeom_calculate_circ_tree(lwg1);
	c2 = lwgeom_calculate_circ_tree(lwg2);
//	printf("\n");
//	circ_tree_print(c1, 0);
//	printf("\n");
//	circ_tree_print(c2, 0);	
//	printf("\n");
	d1 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
	d2 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d3 = circ_tree_distance_tree(c1, c2, &s, threshold);
	d4 = circ_tree_distance_tree(c1, c2, &s, threshold);
//	printf("\n d1-no-tree %20.20g\n d2 %20.20g\n d3 %20.20g\n d4 %20.20g\n", d1, d2, d3, d4);
	circ_tree_free(c1);
	circ_tree_free(c2);
	lwgeom_free(lwg1);
	lwgeom_free(lwg2);
	CU_ASSERT_DOUBLE_EQUAL(d1, d2, 0.00000001);
	CU_ASSERT_DOUBLE_EQUAL(d1, d3, 0.00000001);
	CU_ASSERT_DOUBLE_EQUAL(d1, d4, 0.00000001);

}

static void test_tree_circ_distance_threshold(void)
{
	SPHEROID s;
	int i, j;
	int step = 10;
	
	const char *txt_poly1 = "0103000020E6100000010000000B0000000AA2F068F47651C0F7893DEB70B8454007ABD4C6D57651C000FB650799B84540C21AA2645A7651C011C24BA84AB8454089A9A325E87751C03314EB5453B74540AF9ED96BF57751C0BF9818F889B74540E936A498B47751C0690C87D1C5B74540F5386204DC7751C02FCA658F1AB8454077B65F7B657751C012C586EE37B845408C1862C5977751C00F17E41674B84540D4012F57357751C0AD3BC67E99B845400AA2F068F47651C0F7893DEB70B84540";
	const char *txt_poly2 = "0103000020E610000003000000B5000000E0D13F40187451C01A09009164BB4540771C07D8FA7351C04DBBEDB634BB45400D27D627A47351C070625C9386BB4540D691981FAD7351C02F382CCDFABA4540AA38FA85B97351C03AD3A8271CBB4540D211F0CBBE7351C027F11A8FD9BA4540839AE427EB7351C0BF97C6BBA3BA4540748967A4D47351C00C0037C970BA45408F182A0CF07351C0C050808070BA45405063F06DD27351C0681018D037BA4540E509D5C2EE7351C0B9429E0231BA4540EA39B20CCC7351C018FBB70F22BA454072DD47C8ED7351C04C992D5E0BBA4540934BDE09CE7351C04B17895ACFB94540F7AA9F1EF77351C04DE737D4DCB945400D217458DF7351C022324164B0B945408A2F53C4087451C0A7EC0972B1B9454091A02DF3FC7351C0077F0D388AB94540D9EA0152187451C02316A57280B9454023E37795F07351C09314E67D36B945408DD1866BFC7351C06A0A537214B945405F77D635DE7351C01C7B3C251EB945404785C336F27351C07D767383F1B8454043664E41C77351C0F499D9F3C2B84540DA5CD86FDC7351C0ECA943B295B845403A60B1A2AF7351C063B81C61AFB8454041469468AA7351C0B0727EE16AB845405ADF4611937351C0AA5D2634A3B84540D1BB958D917351C0086C6F1450B845408BBECFF4787351C008F3F4958EB845407F025FE0737351C0506C9F8057B845405A49EDDE5E7351C0AB88DA1B84B845402AAE2CB6767351C07ECC7AEB33B845406C3BCEFF557351C0D93C09506DB8454032C3D6B8577351C0CF4410B115B84540C47F5B30337351C08DCEBB6F7DB845400DC58DB4127351C0D68016845EB845409E0DC353E17251C05E161FDE94B84540D94A5CCDF87251C0D4A79022C3B745404B789041A97151C0D366245447B64540E229E50C7A7151C04864DEC059B645403CF020E75F7151C0C2341E664FB745404F6388C1807151C0BE100BDA2FB7454099DB3D97877151C0853918DE6CB74540080D2824137151C017385A8090B74540A380F14EE97051C0A517F62259B74540C9B0B8E1F47051C019014EF43DB7454075AB809D997051C023033EFC28B745401FCC6CFD3C7051C01778CC51B1B74540A5AFFD2A5E7051C08B7644232EB8454075947311FD7051C0EB4B7B9F09B8454007A084455D7151C065E676C6A7B845401CB4DA891A7151C096D19999D0B84540F9D410BC3E7151C08C3797C11CB945400CA63027017151C0640A96032BBA4540B07FB7D8897151C0B632805859BA454078D01A1FDD7151C0202711EE70B945405F3A6E8E627251C0932D32FE34BA4540B532341D277251C0DFA18E0392BA4540CD8E813B147251C0FEBBA2346EBA4540421F7F16F87151C0DEB93B76A3BA45403B2350A3D37151C0330AF75F99BA454077512147AC7151C0A628D5F117BB454011C9CE77507151C04CC4E1E705BB4540E6889885EE7051C0C45D89CDA8BA4540575D1B37B07051C0F883C0A0D7BA45408A0FB14B977051C01D328086A9BA454033A7D2839B7051C0EEC463DD22BA4540AFDC6C64DC7051C013983A6CE3B94540CC1E39AEE17051C076E753DD7BB94540F6E0B3BECA7051C070E459395BB94540E636719D447051C0CB5C78F2F8B845402D3B6C3AC76F51C08AFD07346AB945404080D345386F51C02EFB7C9465B94540B5BB47ED506F51C0B5742F4FC3B84540F4DAAD32326F51C03AE0604DCEB8454079D03A00286F51C0BFE416D692B84540E7EEF142456F51C026483D9219B84540556E7F645D6F51C06BA1A86620B845406BD12F44536F51C0F976E08663B84540D7BFD8077B6F51C0C500FD8E63B8454004DD9EC4816F51C0441D30FBDBB745403EE733710C6F51C05E6524F44EB74540BEEA42C14A6E51C0367C93EEDEB54540C84C8F841D6E51C0F035740AD0B54540F0B3122E316E51C09158039618B44540F5E629F8706E51C02FD3E2F726B44540533862949E6E51C015FA5534BBB34540C649D8EDEF6E51C02B5E6D6F80B345405D7B912E616F51C0DF4BA76B0EB2454072578CCFB76F51C0A22CCF6690B1454015A5C130647051C0F4EFF7540FB14540F1ACD1FA9D7051C0329CCCA94CB14540374504840E7151C04726DFC9B5B04540C4B031D7D97151C0FC7C331392B04540DC234455E77151C00A9F4A04B9B04540D1505DECCD7151C0D9351517EAB04540CDD86B5EF87151C004CB649085B1454099BE7DDB797151C037D42330B4B245400EA292D32C7151C0820E1DD8B4B24540ACE77E61257151C0EE15BF9BEBB245407AA945D2867151C0666E8B0F01B345404EE1C915D37151C04B6DD883B3B24540C58BDF35247251C0D3FB74C050B445403C06C143697251C0603A1C0D59B44540D2F9E6E6B07251C0BC16F1CCC7B44540E47B911C8D7251C0A95F88542AB5454047BC0FA6237251C031E8A2FB38B54540CAB1FB9FF87151C03B2BF41EF7B445404EBF18E8017251C09C164886A8B4454050CD38EBC67151C014E9B12709B54540B4C7E42DD37151C0FB5B1E864CB54540BECC9D27167251C0B37B1B12AEB545400643207D967251C048C2DBC1ACB5454039070923977251C0E6D50A2602B64540DDD41A1C2E7351C06156AFD91EB645402FAEBC26AD7251C06DB4F5FA28B64540E6498EEFD87251C06214C5327CB645400C5E024B2E7351C0CABD9E93B9B6454001F550AB447351C0FBAFC57D96B645405AE881CF3D7351C01917460EC7B64540695D4AA6607351C00D689CE284B64540C6299985517351C076C84855D3B645406625BA34747351C0E739BF479CB64540386A02DD607351C0D6534476E1B645400B69DB9F997351C0B8AC5ACD90B645400E3CF2EEAC7351C05979D2F9A1B645400FD99F71887351C03916C9ACFAB64540CA154DF19B7351C0CC4ECC370AB745403B8F39FFC07351C0311B27F4ACB6454008ED94DBD07351C069128CC9B9B64540BDEC1CCDAA7351C0FA1AAC0617B74540A99C51C1C07351C07759CE3A2AB7454073FCF177E67351C04A84EAA2CDB6454055CDCB6ED17351C0CB6C7B6438B74540DDD31435F87351C0D4D993891FB74540811059D30D7451C077753AD162B74540F18AF9E46B7451C0FD21E09762B74540F253FA4F547451C067E3343FACB74540F0BCC760697451C0E0AE673BDBB74540FA1616AB497451C0D50836ABF0B74540F41845176E7451C0950B89B7FCB74540A16098ED3D7451C0D4E3E2D80BB84540B9AFBD7B727451C019895C361DB84540163C14EB717451C013C465C245B845402645A98E427451C049F426523FB84540C47E7169707451C0958552ED5DB84540652934E8467451C0D6A4B9DC6EB84540762DC58E717451C0F00F358EA7B8454052BD47144E7451C0CE25A2A9B7B84540FE28AFF55E7451C0EC20702D0BB94540DC47DC52777451C0DB556D5401B945407C974EEE617451C0DF8FB3BF25B94540F486D9A77E7451C0C20621B41AB94540432A3030707451C08AB2EDBC39B945409B0D7D28167551C0BB2B0598F9B84540E9C3E5D1067551C0617DD7107BB94540ADD4816DC57451C052E51C9E7FB9454083A85A09D37451C04AA52729AAB94540E8B74A52A57451C0054656858DB94540C1F58C11C07451C073982E6DC8B945406639F493967451C09D31BA6A90B9454023779107AB7451C026DD7171DFB945404CC088A46D7451C00324607A78B94540B4263081827451C0AF743013AAB94540A1FEA7A25A7451C030846A958DB945403B797701697451C00240A15CE1B945405EBCC1C4497451C0AF0092E9BCB945408A96E72B6A7451C0A85FF698F9B9454048855507427451C06852C7B9F0B94540CFBB01EA5C7451C0DC68FBB715BA4540D42EA3CF3C7451C0D2A6746A0BBA4540510413EB577451C0EE883A7D2CBA4540DC8C76E73B7451C0808E628128BA4540904E729D4F7451C09335C2C358BA4540A9018BEC347451C0017C60D031BA4540508873A32E7451C03F4C9200BDBA45400FEFB0B0567451C00274F969CCBA45402E2B3AC7317451C09C029279CBBA4540C6748AB7417451C060364B9C0EBB4540E0D13F40187451C01A09009164BB4540080000006DABE362BF6F51C03B3EAB0286B84540F2EBCFEEAC6F51C008D8984EA3B84540FF2B5394BB6F51C0B2F08C42CBB84540FDF9E71CFC6F51C07C7C0453C8B845403ABCCFEBCF6F51C0D477C34AC2B845402D81685DC76F51C071D8F5EE94B84540FB229C95E86F51C0BD8E7FF7AEB845406DABE362BF6F51C03B3EAB0286B8454008000000DB185B503E7051C066B728BCD3B245400AC83B92357051C04BA0208DA3B34540D7FCFFA31D7051C0CF7C254CD6B345400E29CDB9907051C012EB7D78CCB34540806300429F7051C0B0D493C19CB34540786CA34E6C7051C093B0BB4060B345407D77C1F9657051C0497CC614C3B24540DB185B503E7051C066B728BCD3B24540";
	const char *polys[2];
	static int npolys = 2;
	
	polys[0] = txt_poly1;
	polys[1] = txt_poly2;

	// spheroid_init(&s, WGS84_RADIUS, WGS84_RADIUS);
	spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

	for ( j = 0; j < npolys; j++ )
	{
		LWGEOM *lwg1 = lwgeom_from_hexwkb(polys[j], LW_PARSER_CHECK_NONE);
		LWGEOM *lwg2 = lwgeom_from_wkt("POINT(-69.83262 43.43636)", LW_PARSER_CHECK_NONE);
	
		CIRC_NODE *c1 = lwgeom_calculate_circ_tree(lwg1);
		CIRC_NODE *c2 = lwgeom_calculate_circ_tree(lwg2);

		for ( i = 50; i < 1500 / step; i++ )
		{
			double d1, d2;
			double d = lwgeom_distance_spheroid(lwg1, lwg2, &s, 0);
			double threshold = step * i;
			d1 = circ_tree_distance_tree(c1, c2, &s, threshold);
			d2 = lwgeom_distance_spheroid(lwg1, lwg2, &s, threshold);
			if (threshold > d && (d1 > threshold || d2 > threshold))
			{
				printf("polygon #%d\n"
		       "threshold = %g\n"
					"true distance = %g\n"
		       "circ_tree_distance = %g\n"
		       "lwgeom_distance_spheroid = %g\n", j, threshold, d, d1, d2);
				CU_FAIL_FATAL();
			}
		}

		circ_tree_free(c1);
		circ_tree_free(c2);
		lwgeom_free(lwg1);
		lwgeom_free(lwg2);
	}
	
}

/*
** Used by test harness to register the tests in this file.
*/
void tree_suite_setup(void);
void tree_suite_setup(void)
{
	CU_pSuite suite = CU_add_suite("Internal Spatial Trees", NULL, NULL);
	PG_ADD_TEST(suite, test_tree_circ_create);
	PG_ADD_TEST(suite, test_tree_circ_pip);
	PG_ADD_TEST(suite, test_tree_circ_pip2);
	PG_ADD_TEST(suite, test_tree_circ_distance);
	PG_ADD_TEST(suite, test_tree_circ_distance_threshold);
}
