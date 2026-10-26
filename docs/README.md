

# Robotics AI Suite Documentation

This directory contains the documentation source and build tooling for the Robotics AI Suite website.

## Architecture

- **Website root (Docusaurus):** Located in `website/`. Docusaurus hosts the repository root site (`/`), navigation, catalog pages, and styling.
- **User guide (Sphinx):** Located in `user-guide/`. The technical documentation and component guides are authored in Markdown and reStructuredText and rendered by Sphinx.
- **Integration:** During site generation, the pre-built Sphinx output (`out/dirhtml`) is staged under `website/.sphinx-static/development-stack/` and served at `/development-stack/`.

## Local Development

### Complete Website

To build and serve the complete unified site (Docusaurus plus Sphinx documentation), run from the repository root:

```bash
make serve
```

For PR-previews or alternate base paths:

```bash
BASE_URL=/pr/2/ PORT=3002 make serve
```

After building with `make website`, run `make test-website` to check the built site with Chromium, Firefox, and WebKit. Install the browsers locally with `npm exec --prefix docs/website -- playwright install chromium firefox webkit`. The test target checks generated pages, responsive layout, client-side model routing, and browser errors without requiring a separate server. If WebKit cannot launch because the host lacks browser libraries, the local test reports it as skipped; CI requires all three browsers.

Browser checks wait for the current shared footer to be inserted after both page loads and client-side model navigation. This avoids navigating away while a remounted footer is still fetching, which Firefox can report as a network error.

On Fedora or another host without Playwright's WebKit dependencies, run `make test-website-container` after `make website`. This uses rootless Podman and the Playwright 1.63.0 Ubuntu image to run all three browsers against the built site, with WebKit required. The container mounts `docs/website` read-only and uses its installed `node_modules`. For alternate base paths, pass the same `BASE_URL` used to build the site, such as `BASE_URL=/pr/2/ make test-website-container`.

### Sphinx User Guide Only

If you are only editing user guide documents and want rapid live reloading of the Sphinx content:

```bash
make -C docs serve
```

This launches `sphinx-autobuild` on the `user-guide/` directory and serves only the documentation pages.

### Available Targets in `docs/`

- `make build`: Build the Sphinx user guide to `out/dirhtml`.
- `make serve`: Launch `sphinx-autobuild` for live editing of Sphinx content.
- `make check`: Create the Python virtual environment and install prerequisite packages.
- `make clean`: Remove Sphinx build artifacts (`out/`).
- `make clean-all`: Remove build artifacts and the Python virtual environment.
- `make help`: Print all available targets and descriptions.

## Authenticating Sync Workflows with AWS

To authenticate the website deployment workflow with AWS, you need to update your AWS IAM 

IAM->Access Management->Roles->(your role)->Trust relationships

Information on setting up your trust relationship can be found in [GitHub Docs](https://docs.github.com/en/actions/how-tos/secure-your-work/security-harden-deployments/oidc-in-aws#configuring-the-role-and-trust-policy)


### Using GH CLI to Fetch Repo and Org IDs

You can use the GH CLI to fetch the repository and organization ids that will be needed to authorize the trust relationship in AWS:
```
gh api repos/open-edge-platform/robotics-ai-suite --jq '{repository_id: .id, full_name: .full_name, owner_id: .owner.id, owner_login: .owner.login, owner_type: .owner.type}' && gh api orgs/open-edge-platform --jq '{organization_id: .id, login: .login}'
```