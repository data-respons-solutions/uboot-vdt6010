#ifdef CONFIG_NXP_BOARD_REVISION
int nxp_board_rev(void)
{
	/*
	 * Get Board ID information from OCOTP_GP1[15:8]
	 * RevA: 0x1
	 * RevB: 0x2
	 * RevC: 0x3
	 */
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
			(struct fuse_bank4_regs *)bank->fuse_regs;

	return (readl(&fuse->gp1) >> 8 & 0x0F);
}

char nxp_board_rev_string(void)
{
	const char *rev = "A";

	return (*rev + nxp_board_rev() - 1);
}
