# Haven shared rental portal

The webpage supports 25 rooms and uses Supabase Auth with a shared Postgres database. Monthly rent is ₹6,200 for rooms 1–8, ₹5,200 for rooms 9–18, ₹5,000 for rooms 19 and 21–24, ₹4,500 for room 20, and ₹4,000 for room 25. Tenant registrations and contact changes are visible to the owner; payment submissions are also shared and can be approved or rejected by the owner. Database row-level security limits tenants to their own records.

## Connect Supabase

1. Create a Supabase project.
2. In the Supabase SQL Editor, run [`supabase-schema.sql`](supabase-schema.sql). If you already ran the earlier 18-room schema, run this updated file again to enable rooms 19–25.
3. In **Project Settings → API** (or the project's Connect dialog), copy the **Project URL** and **publishable key**. Put those values in [`supabase-config.js`](supabase-config.js), replacing the placeholders.
4. In Supabase **Authentication → URL Configuration**, add the eventual GitHub Pages URL to the allowed redirect URLs. Email confirmation can stay enabled; new tenants finish registration after confirming and signing in.
5. Create the owner account from the Supabase Dashboard under **Authentication → Users**. After creating it, copy that user's UUID and run this in the SQL Editor:

   ```sql
   insert into public.owner_accounts (user_id)
   values ('PASTE_OWNER_USER_UUID');
   ```

   Only add the owner account through the SQL Editor. Accounts registered on the webpage are tenant accounts.

## Publish with GitHub Pages

1. Add `index.html`, `supabase-config.js`, and `supabase-schema.sql` to the root of a GitHub repository.
2. In the repository, open **Settings → Pages**.
3. Choose **Deploy from a branch**, select your default branch and the `/ (root)` folder, then save.
4. Add the published URL from GitHub Pages to Supabase's allowed redirect URLs.

The Supabase publishable key is designed to be used in a browser. Never put a Supabase `service_role` key, database password, or private credential in this repository. The SQL enables row-level security: tenant accounts can read and update their own profile and submit rent details; the owner account can read tenant records and review payment submissions.

The payment form saves a screenshot filename only. It does not upload an image. To store payment screenshots, configure a private Supabase Storage bucket and its access policies separately.
